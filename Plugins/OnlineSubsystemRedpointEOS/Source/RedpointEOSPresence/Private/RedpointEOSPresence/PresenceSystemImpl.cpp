// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceSystemImpl.h"

REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()
#include "RedpointEOSCompat/OptionalCompat.h"
REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END()

#include "RedpointEOSAPI/ActiveSession/CopyInfo.h"
#include "RedpointEOSAPI/Sessions/CopyActiveSessionHandle.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/FireWithin.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSCore/Utils/SetArray.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPresence/Logging.h"
#include "RedpointEOSPresence/PresenceUserStateSerialization.h"
#include "RedpointEOSPresence/Stats.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeature.h"
#include "RedpointEOSRooms/Features/MatchStateRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeature.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeature.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeature.h"
#include "RedpointEOSRooms/Requests/CreateRoomRequest.h"
#include "RedpointEOSRooms/Requests/LeaveRoomRequest.h"
#include "RedpointEOSRooms/Requests/UpdateRoomRequest.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#if WITH_EDITOR
#include "Internationalization/TextLocalizationManager.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3969572298, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::API::Sessions;
using namespace ::Redpoint::EOS::API::ActiveSession;

FPresenceSystemImpl::FPresenceSystemImpl(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , LocalUserStates()
    , SyncQueue(MakeShared<FExclusiveAsyncOperationQueue>())
    , LocalTimer(MakeShared<FPeriodicTimer>(PeriodicSecondsForLocalUserCheck))
    , RemoteTimer(MakeShared<FPeriodicTimer>(PeriodicSecondsForRemoteUserRefresh))
    , RemoteUserPresenceStateUpdatedDelegates()
    , InboundMessageConnectionEstablishedDelegates()
    , OnRoomJoinedHandle()
    , OnRoomUpdatedHandle()
    , OnRoomLeftHandle()
    , DeferredInterestCallbacks()
    , PendingRemoteRefreshCallbacks(0)
    , InternetConnectivityChangedDelegate()
    , bDisconnectedFromInternet(false)
{
}

void FPresenceSystemImpl::RegisterEvents()
{
    this->LocalTimer->Start(
        FPeriodicTimerDelegate::CreateSP(this, &FPresenceSystemImpl::ScheduleDesiredPresenceUpdatesWhereOutOfDate));
    this->RemoteTimer->Start(
        FPeriodicTimerDelegate::CreateSP(this, &FPresenceSystemImpl::ScheduleRemotePresenceRefreshForLocalUsers));

    auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();
    this->OnRoomJoinedHandle = RoomSystem->OnRoomJoined(NAME_Game).AddSP(this, &FPresenceSystemImpl::OnRoomJoined);
    this->OnRoomUpdatedHandle = RoomSystem->OnRoomUpdated(NAME_Game).AddSP(this, &FPresenceSystemImpl::OnRoomUpdated);
    this->OnRoomLeftHandle = RoomSystem->OnRoomLeft(NAME_Game).AddSP(this, &FPresenceSystemImpl::OnRoomLeft);
}

void FPresenceSystemImpl::UnregisterEvents()
{
    this->RemoteTimer->Stop();
    this->LocalTimer->Stop();

    auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();
    RoomSystem->OnRoomJoined(NAME_Game).Remove(this->OnRoomJoinedHandle);
    RoomSystem->OnRoomUpdated(NAME_Game).Remove(this->OnRoomUpdatedHandle);
    RoomSystem->OnRoomLeft(NAME_Game).Remove(this->OnRoomLeftHandle);
}

void FPresenceSystemImpl::AddLocalUser(
    const FIdentityUserRef &InLocalUser,
    const FOnLocalUserAdded &InCompletionDelegate)
{
    const auto LocalUserId = InLocalUser->GetUserId();

    if (!IsProductUserId(LocalUserId))
    {
        // Ignore local users that aren't associated with a PUID (e.g. dedicated servers).
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    if (this->LocalUserStates.Contains(LocalUserId))
    {
        // This user is already tracked, which can happen if the identity system fired events in response to account
        // linking.
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

#if WITH_EDITOR
    // @note: We need to do this so that the editor loads game translations (from things like blueprints), so that
    // FText::FindText below works.
    FTextLocalizationManager::Get().EnableGameLocalizationPreview();
#endif

    // Create presence state for this local user.
    UE_LOG(
        LogRedpointEOSPresence,
        Verbose,
        TEXT("Local user %s is being registered with cross-platform presence..."),
        *GetUserIdString(LocalUserId));
    auto &LocalUserState = this->LocalUserStates.Add(
        LocalUserId,
        MakeShared<FPresenceSystemLocalUserState>(this->PlatformHandle, LocalUserId, InLocalUser->GetUserSlot()));
    LocalUserState->OnRemoteConnectionReceivedDelegate.AddSP(
        this,
        &FPresenceSystemImpl::OnRemoteConnectionReceived,
        LocalUserState.ToSharedRef());
    LocalUserState->OnNonUnifiedFriendPresenceChangeRequiresBroadcast =
        TDelegate<void(const FIdentityUserId &, const FPresenceUserState &)>::CreateSPLambda(
            this,
            [this, LocalUserId](const FIdentityUserId &RemoteUserId, const FPresenceUserState &NewPresenceState) {
                auto LocalUserStateOnCallback = this->LocalUserStates.FindRef(LocalUserId);
                if (!LocalUserStateOnCallback.IsValid())
                {
                    return;
                }

                auto *RemoteUserState = LocalUserStateOnCallback->RemoteUsers.Find(RemoteUserId);
                if (RemoteUserState == nullptr)
                {
                    return;
                }

                for (const auto &InterestedSystem : RemoteUserState->InterestedSystems)
                {
                    UE_LOG(
                        LogRedpointEOSPresence,
                        VeryVerbose,
                        TEXT("Broadcasting OnRemoteUserPresenceStateUpdated event for interested system '%s' for local "
                             "user ID '%s' and remote presence user ID '%s', with basic activity state from runtime "
                             "platform integrations."),
                        *InterestedSystem.ToString(),
                        *GetUserIdString(LocalUserId),
                        *GetUserIdString(RemoteUserId));
                    this->OnRemoteUserPresenceStateUpdated(InterestedSystem)
                        .Broadcast(LocalUserId, RemoteUserId, NewPresenceState);
                }
            });
    LocalUserState->OnDisconnectionStateChanged =
        FSimpleDelegate::CreateSP(this, &FPresenceSystemImpl::RecomputeInternetConnectivityState);
    LocalUserState->Init(FPresenceSystemLocalUserState::FOnInitComplete::CreateSPLambda(
        this,
        [this, LocalUserId, InCompletionDelegate](const FError &ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                this->LocalUserStates.Remove(LocalUserId);
            }
            InCompletionDelegate.ExecuteIfBound(ErrorCode);
        }));
}

void FPresenceSystemImpl::RemoveLocalUser(
    const FIdentityUserRef &InLocalUser,
    const FOnLocalUserRemoved &InCompletionDelegate)
{
    const auto LocalUserId = InLocalUser->GetUserId();

    if (!IsProductUserId(LocalUserId))
    {
        // Ignore local users that aren't associated with a PUID (e.g. dedicated servers).
        InCompletionDelegate.ExecuteIfBound();
        return;
    }

    auto *LocalUserState = this->LocalUserStates.Find(LocalUserId);
    if (LocalUserState == nullptr || !(*LocalUserState).IsValid())
    {
        if (LocalUserState != nullptr)
        {
            (*LocalUserState)->OnRemoteConnectionReceivedDelegate.Clear();
            (*LocalUserState)->OnNonUnifiedFriendPresenceChangeRequiresBroadcast.Unbind();
            this->LocalUserStates.Remove(LocalUserId);
        }
        UE_LOG(
            LogRedpointEOSPresence,
            Verbose,
            TEXT("Ignoring request to remove local user %s from cross-platform presence because they're already not "
                 "tracked by cross-platform presence."),
            *GetUserIdString(LocalUserId));
        InCompletionDelegate.ExecuteIfBound();
        return;
    }

    UE_LOG(
        LogRedpointEOSPresence,
        Verbose,
        TEXT("Local user %s is being unregistered from cross-platform presence..."),
        *GetUserIdString(LocalUserId));

    (*LocalUserState)
        ->Shutdown(FPresenceSystemLocalUserState::FOnShutdownComplete::CreateSPLambda(
            this,
            [this, LocalUserId, InCompletionDelegate](const FError &ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogRedpointEOSPresence,
                        Warning,
                        TEXT("Unable to remove cross-platform presence lobby for local user %s: %s"),
                        *GetUserIdString(LocalUserId),
                        *ErrorCode.ToLogString());
                }

                this->LocalUserStates.Remove(LocalUserId);
                InCompletionDelegate.ExecuteIfBound();
                return;
            }));
}

IPresenceSystem::FOnRemoteUserPresenceStateUpdated &FPresenceSystemImpl::OnRemoteUserPresenceStateUpdated(
    const FName &InterestedSystem)
{
    return this->RemoteUserPresenceStateUpdatedDelegates.FindOrAdd(InterestedSystem);
}

void FPresenceSystemImpl::UpdatePresenceInterest(
    const FName &InterestedSystem,
    const FIdentityUserId &LocalUserId,
    const TArray<FPresenceInterestChange> &TargetUserInterest,
    bool bWaitForInitialPresenceData,
    const FOnUpdatePresenceInterestComplete &CompletionDelegate)
{
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    if (!LocalUserState.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("This local user is not tracked in cross-platform presence.")),
            TMap<FIdentityUserId, FPresenceUserState>());
        return;
    }

    auto UserCacheSystem = this->PlatformHandle->GetSystem<IUserCacheSystem>();

    // Update RemoteUsers and detect if we're making any changes.
    TSet<FIdentityUserId> InterestedUserIds;
    FRuntimePlatformServiceCallContextPtr CallContext = nullptr;
    for (const auto &InterestChange : TargetUserInterest)
    {
        auto *TargetUserState = LocalUserState->RemoteUsers.Find(InterestChange.TargetUserId);
        if (TargetUserState == nullptr)
        {
            if (!InterestChange.bInterested)
            {
                // Target user isn't tracked and we're not interested, no change.
                continue;
            }
            else
            {
                // Add new state for this remote user.
                UE_LOG(
                    LogRedpointEOSPresence,
                    Verbose,
                    TEXT("System '%s' is now interested in remote user '%s'."),
                    *InterestedSystem.ToString(),
                    *GetUserIdString(InterestChange.TargetUserId));
                TargetUserState = &LocalUserState->RemoteUsers.Add(
                    InterestChange.TargetUserId,
                    FPresenceSystemRemoteUserState(InterestChange.TargetUserId));
            }
        }

        bool bCurrentSystemIsAlreadyInterested = TargetUserState->InterestedSystems.Contains(InterestedSystem);
        if (bCurrentSystemIsAlreadyInterested && !InterestChange.bInterested)
        {
            UE_LOG(
                LogRedpointEOSPresence,
                Verbose,
                TEXT("System '%s' is no longer interested in remote user '%s'."),
                *InterestedSystem.ToString(),
                *GetUserIdString(InterestChange.TargetUserId));
            TargetUserState->InterestedSystems.Remove(InterestedSystem);
            if (TargetUserState->InterestedSystems.Num() == 0)
            {
                // No interested systems left, remove this entry.
                LocalUserState->RemoteUsers.Remove(InterestChange.TargetUserId);
                continue;
            }
        }
        else if (!bCurrentSystemIsAlreadyInterested && InterestChange.bInterested)
        {
            TargetUserState->InterestedSystems.Add(InterestedSystem);
        }

        if (InterestChange.bInterested)
        {
            InterestedUserIds.Add(InterestChange.TargetUserId);
        }
    }

    // If we're not waiting for initial presence, we're done.
    if (!bWaitForInitialPresenceData)
    {
        CompletionDelegate.ExecuteIfBound(Errors::Success(), TMap<FIdentityUserId, FPresenceUserState>());
        return;
    }

    // If we're waiting for initial presence, but all the users we're interested in already have initial presence data,
    // return that immediately.
    bool bHasAllInitialPresenceData = true;
    TMap<FIdentityUserId, FPresenceUserState> InitialPresenceData;
    for (const auto &TargetUserId : InterestedUserIds)
    {
        auto &TargetUserState = LocalUserState->RemoteUsers.FindChecked(TargetUserId);
        if (TargetUserState.FullPresence.IsSet())
        {
            InitialPresenceData.Add(TargetUserId, *TargetUserState.GetBestPresence());
        }
        else
        {
            bHasAllInitialPresenceData = false;
            break;
        }
    }
    if (bHasAllInitialPresenceData)
    {
        CompletionDelegate.ExecuteIfBound(Errors::Success(), InitialPresenceData);
        return;
    }

    // Defer the callback.
    this->DeferredInterestCallbacks.Add(
        FPresenceSystemDeferredInterestCallback(LocalUserId, InterestedUserIds, CompletionDelegate));
    this->ScheduleRemotePresenceRefreshForLocalUser(LocalUserId);
    return;
}

TOptional<FPresenceUserState> FPresenceSystemImpl::GetPresence(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &TargetUserId) const
{
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    if (!LocalUserState.IsValid())
    {
        return TOptional<FPresenceUserState>();
    }

    if (TargetUserId == LocalUserId)
    {
        return LocalUserState->DesiredPresence;
    }

    auto *RemoteUserStatePtr = LocalUserState->RemoteUsers.Find(TargetUserId);
    if (RemoteUserStatePtr == nullptr)
    {
        return TOptional<FPresenceUserState>();
    }

    return RemoteUserStatePtr->GetBestPresence();
}

void FPresenceSystemImpl::SetPresence(
    const FIdentityUserId &LocalUserId,
    const FPresenceUserState &NewPresenceState,
    const FOnSetPresenceComplete &CompletionDelegate)
{
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    if (!LocalUserState.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(Errors::NotFound(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("This local user is not tracked in cross-platform presence.")));
        return;
    }

    if (GetTypeHash(LocalUserState->CurrentPresence) == GetTypeHash(NewPresenceState) &&
        GetTypeHash(LocalUserState->DesiredPresence) == GetTypeHash(NewPresenceState))
    {
        // Fast return, since there won't be an update.
        CompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    // Update desired presence data.
    this->UpdateDesiredPresenceAdvertised(LocalUserState);
    LocalUserState->DesiredPresence.ActivityState = NewPresenceState.ActivityState;
    LocalUserState->DesiredPresence.StatusTextFormat = NewPresenceState.StatusTextFormat;
    LocalUserState->DesiredPresence.StatusTextFormatArguments = NewPresenceState.StatusTextFormatArguments;
    LocalUserState->DesiredPresence.CustomAttributes = NewPresenceState.CustomAttributes;

    // Increment scheduled presence update count.
    LocalUserState->LastScheduledPresenceUpdate += 1;
    auto ThisPresenceUpdate = LocalUserState->LastScheduledPresenceUpdate;

    // Schedule a presence update, and wait for it to happen.
    TSharedRef<FDelegateHandle> DelegateHandlePtr = MakeShared<FDelegateHandle>();
    *DelegateHandlePtr =
        LocalUserState->OnDesiredPresenceProcessedDelegate.Add(FOnDesiredPresenceProcessed::FDelegate::CreateSPLambda(
            this,
            [DelegateHandlePtr, LocalUserState, ThisPresenceUpdate, CompletionDelegate](
                uint32 JustProcessedPresenceUpdate,
                const FError &ErrorCode) {
                if (JustProcessedPresenceUpdate == ThisPresenceUpdate)
                {
                    CompletionDelegate.ExecuteIfBound(ErrorCode);
                    LocalUserState->OnDesiredPresenceProcessedDelegate.Remove(*DelegateHandlePtr);
                }
            }));
    this->SyncQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSP(
        this,
        &FPresenceSystemImpl::OnProcessRequestedLocalUserUpdate,
        LocalUserState->LocalUserId,
        LocalUserState.ToSharedRef(),
        ThisPresenceUpdate));
}

void FPresenceSystemImpl::UpdateDesiredPresenceAdvertised(
    const TSharedPtr<FPresenceSystemLocalUserState> &LocalUserState)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();

    // Get the runtime integrations, and see if any of them want to turn off presence.
    bool bPresenceAdvertised = true;
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        auto SignedInUser = IdentitySystem->GetUser(LocalUserState->LocalUserId);
        if (SignedInUser.IsValid())
        {
            auto CallContext =
                ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, SignedInUser->GetUserSlot());

            TOptional<bool> bLocalUserOnline = RuntimeIntegration->GetPresenceService()->IsLocalUserOnline(CallContext);
            if (bLocalUserOnline.IsSet())
            {
                bPresenceAdvertised = bLocalUserOnline.GetValue();
                break;
            }
        }
    }

    // Assign presence advertised value.
    if (LocalUserState->DesiredPresence.bPresenceAdvertised != bPresenceAdvertised)
    {
        UE_LOG(
            LogRedpointEOSPresence,
            Verbose,
            TEXT("Local user %s is has changed presence advertisement state: %s"),
            *GetUserIdString(LocalUserState->LocalUserId),
            bPresenceAdvertised ? TEXT("advertised") : TEXT("hidden"));
    }
    LocalUserState->DesiredPresence.bPresenceAdvertised = bPresenceAdvertised;
}

void FPresenceSystemImpl::ScheduleDesiredPresenceUpdatesWhereOutOfDate()
{
    // Compare current and desired presence states, and queue operations to update presence for the
    // ones that are out-of-date.
    for (auto &LocalUserStateKV : this->LocalUserStates)
    {
        auto &LocalUserState = LocalUserStateKV.Value;

        this->UpdateDesiredPresenceAdvertised(LocalUserState);

        if (GetTypeHash(LocalUserState->DesiredPresence) != GetTypeHash(LocalUserState->CurrentPresence))
        {
            UE_LOG(
                LogRedpointEOSPresence,
                VeryVerbose,
                TEXT("Scheduling cross-platform presence update for local user %s as their desired presence state and "
                     "current presence state are out-of-sync."),
                *GetUserIdString(LocalUserState->LocalUserId));
            LocalUserState->LastScheduledPresenceUpdate += 1;
            auto ThisPresenceUpdate = LocalUserState->LastScheduledPresenceUpdate;
            this->SyncQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSP(
                this,
                &FPresenceSystemImpl::OnProcessRequestedLocalUserUpdate,
                LocalUserState->LocalUserId,
                LocalUserState.ToSharedRef(),
                ThisPresenceUpdate));
        }
    }
}

void FPresenceSystemImpl::OnProcessRequestedLocalUserUpdate(
    const FExclusiveAsyncOperationQueue::FOnDone &InOnDone,
    FIdentityUserId InLocalUserId,
    TSharedRef<FPresenceSystemLocalUserState> InFallbackStateForSignalling,
    uint32 InThisPresenceUpdate)
{
    // Get the current slot for this local user.
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto LocalUser = IdentitySystem->GetUser(InLocalUserId);
    if (!LocalUser.IsValid())
    {
        // This local user is no longer signed in.
        InFallbackStateForSignalling->ProcessedPresenceUpdate++;
        InFallbackStateForSignalling->OnDesiredPresenceProcessedDelegate.Broadcast(
            InThisPresenceUpdate,
            Errors::NoConnection());
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Local user %s is no longer registered with cross-platform presence by the time "
                 "OnProcessRequestedUpdate ran."),
            *GetUserIdString(InLocalUserId));
        InOnDone.ExecuteIfBound();
        return;
    }

    // Create the call context for runtime platform integrations.
    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, LocalUser->GetUserSlot());

    // Check that this local user is still tracked.
    auto *LocalUserStatePtr = this->LocalUserStates.Find(InLocalUserId);
    if (LocalUserStatePtr == nullptr || !LocalUser.IsValid())
    {
        // This local user is no longer registered with presence.
        InFallbackStateForSignalling->ProcessedPresenceUpdate++;
        InFallbackStateForSignalling->OnDesiredPresenceProcessedDelegate.Broadcast(
            InThisPresenceUpdate,
            Errors::NoConnection());
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Local user %s is no longer registered with cross-platform presence by the time "
                 "OnProcessRequestedUpdate ran."),
            *GetUserIdString(InLocalUserId));
        InOnDone.ExecuteIfBound();
        return;
    }

    // Check that this local user is still out-of-date.
    auto &LocalUserState = *LocalUserStatePtr;
    checkf(
        LocalUserState->ProcessedPresenceUpdate == InThisPresenceUpdate - 1,
        TEXT("Unexpected presence update skip!"));
    if (GetTypeHash(LocalUserState->DesiredPresence) == GetTypeHash(LocalUserState->CurrentPresence))
    {
        // This local user is already up-to-date.
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Local user %s is already up-to-date in cross-platform presence by the time "
                 "OnProcessRequestedUpdate ran."),
            *GetUserIdString(InLocalUserId));
        LocalUserState->ProcessedPresenceUpdate++;
        LocalUserState->OnDesiredPresenceProcessedDelegate.Broadcast(InThisPresenceUpdate, Errors::Success());
        InOnDone.ExecuteIfBound();
        return;
    }

    // Create a snapshot of the desired state. This operation might take multiple frames to update the underlying room,
    // and desired state might change in the meantime, so we need to snapshot to ensure that the current local state
    // gets set to what we updated to, not what is desired in several frames time.
    auto DesiredLocalUserStateSnapshot = LocalUserState->DesiredPresence;

    // Create the execution graph since we need to perform asynchronous operations for both the EOS cross-platform
    // presence room and on the runtime integrations.
    auto ExecutionGraph =
        MakeShared<FAsyncTaskGraph>(GET_STATID(STAT_RedpointEOS_Presence_OnProcessRequestedUpdateGraph));

    // Compute and execute the room update request for the cross-platform presence room.
    TArray<FAsyncTaskRef> RequiredTasks;
    auto CrossPlatformTask = ExecutionGraph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [LocalUserState, DesiredLocalUserStateSnapshot](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                // Ask the local user state to create or update the cross-platform presence room. We delegate this work
                // to the local user state because the cross-platform presence lobby could have disappeared due to
                // Internet connectivity loss.
                LocalUserState->UpdatePresenceRoomWithDesiredSnapshot(DesiredLocalUserStateSnapshot, OnTaskComplete);
            }),
        FAsyncTaskGraph::ENoDependencies::StartImmediately);
    RequiredTasks.Add(CrossPlatformTask);

    // Schedule calls to the presence runtime integration as well, but only execute them if the cross-platform presence
    // update succeeds.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        // Perform general update.
        auto IntegrationTask = ExecutionGraph->AddDelegate(
            FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
                this,
                [this, RuntimeIntegration, LocalUser, DesiredLocalUserStateSnapshot, CallContext](
                    const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                    RuntimeIntegration->GetPresenceService()->UpdateLocalUserPresence(
                        CallContext,
                        DesiredLocalUserStateSnapshot,
                        FireWithin(
                            10.0f,
                            IRuntimePlatformPresenceService::FOnLocalUserPresenceUpdated::CreateSPLambda(
                                this,
                                [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                    LocalUser,
#endif
                                    OnTaskComplete](FError ErrorCode) {
                                    if (!ErrorCode.WasSuccessful() &&
                                        ErrorCode.ResultCode() != EOS_EResult::EOS_NotImplemented)
                                    {
                                        if (!GIsAutomationTesting)
                                        {
                                            UE_LOG(
                                                LogRedpointEOSPresence,
                                                Warning,
                                                TEXT("Runtime integration for presence failed to update for local "
                                                     "user '%s': %s"),
                                                *GetUserIdString(LocalUser->GetUserId()),
                                                *ErrorCode.ToLogString());
                                        }
                                    }
                                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                                }),
                            Errors::TimedOut(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("Runtime platform integration timed out while updating general presence "
                                     "information."))));
                }),
            CrossPlatformTask);
        RequiredTasks.Add(IntegrationTask);

        // Factory for generating delegates since the logic is the same for AdvertisedSession and AdvertisedParty.
        auto AddAdvertisementDelegates =
            [&](TFunction<const TOptional<FPresenceUserAdvertisedRoom> &(const FPresenceUserState &)> Accessor,
                bool bIsParty) {
                // Delete the existing room advertisement if we are now advertising a new room.
                auto DeleteTask = ExecutionGraph->AddDelegate(
                    FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
                        this,
                        [this,
                         RuntimeIntegration,
                         LocalUser,
                         LocalUserState,
                         DesiredLocalUserStateSnapshot,
                         Accessor,
                         CallContext,
                         bIsParty](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                            // If we don't currently have a room advertisement, or
                            // if are currently advertising but it's still the same room,
                            // then there's nothing to do.
                            bool bHasAdvertiseableCurrentRoom = Accessor(LocalUserState->CurrentPresence).IsSet();
                            bool bHasAdvertiseableDesiredRoom = Accessor(DesiredLocalUserStateSnapshot).IsSet();
                            if (!bHasAdvertiseableCurrentRoom ||
                                (bHasAdvertiseableDesiredRoom &&
                                 Accessor(DesiredLocalUserStateSnapshot)
                                     ->RoomId->IsEqual(Accessor(LocalUserState->CurrentPresence)->RoomId)))
                            {
                                OnTaskComplete.ExecuteIfBound(Errors::Success());
                                return;
                            }

                            auto CurrentRoomId = Accessor(LocalUserState->CurrentPresence)->RoomId;

                            RuntimeIntegration->GetPresenceService()->DeleteRoomAdvertisement(
                                CallContext,
                                CurrentRoomId,
                                bIsParty,
                                FireWithin(
                                    10.0f,
                                    IRuntimePlatformPresenceService::FOnDeleteRoomAdvertisementComplete::CreateSPLambda(
                                        this,
                                        [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                            CurrentRoomId,
#endif
                                            OnTaskComplete](FError ErrorCode) {
                                            if (!ErrorCode.WasSuccessful() &&
                                                ErrorCode.ResultCode() != EOS_EResult::EOS_NotImplemented)
                                            {
                                                if (!GIsAutomationTesting)
                                                {
                                                    UE_LOG(
                                                        LogRedpointEOSPresence,
                                                        Warning,
                                                        TEXT("Runtime integration for presence failed to delete "
                                                             "existing room advertisement for room '%s': %s"),
                                                        *CurrentRoomId->ToString(),
                                                        *ErrorCode.ToLogString());
                                                }
                                            }
                                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                                        }),
                                    Errors::TimedOut(
                                        ANSI_TO_TCHAR(__FUNCTION__),
                                        TEXT("Runtime platform integration timed out while deleting existing room "
                                             "advertisement."))));
                        }),
                    TArray<FAsyncTaskRef>{CrossPlatformTask, IntegrationTask});
                RequiredTasks.Add(DeleteTask);

                // Create, join or update the current room advertisement if we're advertising a room.
                auto UpsertTask = ExecutionGraph->AddDelegate(
                    FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
                        this,
                        [this,
                         RuntimeIntegration,
                         LocalUser,
                         LocalUserState,
                         DesiredLocalUserStateSnapshot,
                         Accessor,
                         CallContext,
                         bIsParty](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                            // If we don't have a desired room advertisement, then there's nothing to do.
                            if (!Accessor(DesiredLocalUserStateSnapshot).IsSet())
                            {
                                OnTaskComplete.ExecuteIfBound(Errors::Success());
                                return;
                            }

                            auto DesiredRoomId = Accessor(DesiredLocalUserStateSnapshot)->RoomId;

                            RuntimeIntegration->GetPresenceService()->UpsertRoomAdvertisement(
                                CallContext,
                                Accessor(DesiredLocalUserStateSnapshot).GetValue(),
                                bIsParty,
                                FireWithin(
                                    10.0f,
                                    IRuntimePlatformPresenceService::FOnDeleteRoomAdvertisementComplete::CreateSPLambda(
                                        this,
                                        [DesiredRoomId, OnTaskComplete](FError ErrorCode) {
                                            if (!ErrorCode.WasSuccessful() &&
                                                ErrorCode.ResultCode() != EOS_EResult::EOS_NotImplemented)
                                            {
                                                if (!GIsAutomationTesting)
                                                {
                                                    UE_LOG(
                                                        LogRedpointEOSPresence,
                                                        Warning,
                                                        TEXT("Runtime integration for presence failed to upsert room "
                                                             "advertisement for room '%s': %s"),
                                                        *DesiredRoomId->ToString(),
                                                        *ErrorCode.ToLogString());
                                                }
                                            }
                                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                                        }),
                                    Errors::TimedOut(
                                        ANSI_TO_TCHAR(__FUNCTION__),
                                        TEXT("Runtime platform integration timed out while upserting room "
                                             "advertisement."))));
                        }),
                    TArray<FAsyncTaskRef>{CrossPlatformTask, IntegrationTask, DeleteTask});
                RequiredTasks.Add(UpsertTask);
            };

        // Add the delegates for both advertised sessions and advertised parties.
        //
        // @note: This check prevents the presence system calling UpsertRoomAdvertisement for sessions until the room
        // system supports sessions. This allows FSyntheticSessionManager in the OSSv1 implementation to directly call
        // UpsertRoomAdvertisement to manage session advertisement through runtime platform integrations without
        // conflicts.
        if (this->PlatformHandle->GetSystem<IRoomSystem>()->HasRoomProvider(FName(TEXT("Session"))))
        {
            AddAdvertisementDelegates(
                [](const FPresenceUserState &P) -> const TOptional<FPresenceUserAdvertisedRoom> & {
                    return P.AdvertisedSession;
                },
                false);
        }
        AddAdvertisementDelegates(
            [](const FPresenceUserState &P) -> const TOptional<FPresenceUserAdvertisedRoom> & {
                return P.AdvertisedParty;
            },
            true);
    }

    // Update the local user's current presence to match the desired presence we've now updated to.
    auto UpdateLocalStateTasks = ExecutionGraph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                InLocalUserId,
#endif
                LocalUserState,
                DesiredLocalUserStateSnapshot](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(
                    LogRedpointEOSPresence,
                    VeryVerbose,
                    TEXT("Updating the current local presence state for local user %s after "
                         "OnProcessRequestedUpdate completed."),
                    *GetUserIdString(InLocalUserId));
                LocalUserState->CurrentPresence = DesiredLocalUserStateSnapshot;
                OnTaskComplete.ExecuteIfBound(Errors::Success());
            }),
        RequiredTasks);

    // Finally, signal that this operation is done.
    ExecutionGraph->Finally([InOnDone, InThisPresenceUpdate, LocalUserState](const FError &ErrorCode) {
        checkf(
            LocalUserState->ProcessedPresenceUpdate == InThisPresenceUpdate - 1,
            TEXT("Unexpected presence update skip after room update!"));
        LocalUserState->ProcessedPresenceUpdate++;
        LocalUserState->OnDesiredPresenceProcessedDelegate.Broadcast(InThisPresenceUpdate, ErrorCode);
        InOnDone.ExecuteIfBound();
    });
    ExecutionGraph->Start();
}

void FPresenceSystemImpl::OnRoomJoined(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomJoinReason Reason)
{
    this->OnUpdateDesiredPresenceStateForRooms();
}

void FPresenceSystemImpl::OnRoomUpdated(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    const FRoomUpdateInfo &RoomUpdateInfo)
{
    this->OnUpdateDesiredPresenceStateForRooms();
}

void FPresenceSystemImpl::OnRoomLeft(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomLeaveReason Reason)
{
    this->OnUpdateDesiredPresenceStateForRooms();
}

FPresenceUserAdvertisedRoom FPresenceSystemImpl::CreateAdvertisedRoomFromRoom(const FRoomRef &Room)
{
    auto State = FPresenceUserAdvertisedRoom(Room->GetRoomId(), Room->GetRoomNamespace());
    State.CurrentMemberCount = static_cast<uint32>(Room->GetMembers().Num());
    State.MaximumMemberCount = Room->GetRoomMaxMembers();

    // For a room to be joinable by presence, it needs to meet two requirements:
    // - The room has presence and "join by presence" enabled, which facilitiates presence joins via the Epic Games
    // overlay.
    // - The room is either publicly advertised or has the "join by ID" feature enabled, which allows presence joins
    // from runtime platforms (which inheritantly need to search for the room due to limitations on the way the EOS SDK
    // works).
    auto PresenceFeature = Room->GetRoomFeature<IPresenceRoomFeature>();
    auto PublicAdvertisementFeature = Room->GetRoomFeature<IPublicAdvertisementRoomFeature>();
    auto JoinByIdFeature = Room->GetRoomFeature<IJoinByIdRoomFeature>();
    if (PresenceFeature.IsValid() && PresenceFeature->AllowJoinFromPresence())
    {
        if (PublicAdvertisementFeature.IsValid())
        {
            State.bJoinable = true;
        }
        else if (JoinByIdFeature.IsValid())
        {
            State.bJoinable = true;
        }
        else
        {
            State.bJoinable = false;
        }
    }
    else
    {
        State.bJoinable = false;
    }

    auto PartyConfigurationFeature = Room->GetRoomFeature<IPartyConfigurationRoomFeature>();
    if (PartyConfigurationFeature)
    {
        State.SecretPassword = PartyConfigurationFeature->GetPassword();
        State.bRequiresSecretPassword = !PartyConfigurationFeature->GetPassword().IsEmpty();
    }

    auto MatchFeature = Room->GetRoomFeature<IMatchStateRoomFeature>();
    if (MatchFeature)
    {
        State.bIsMatch = true;
        State.bMatchStarted = false; /* @todo */
    }

    return State;
}

void FPresenceSystemImpl::OnUpdateDesiredPresenceStateForRooms()
{
    // Get the room system, and for all local users, check what rooms we're in
    // and which ones are advertising either as a game session or as a party.
    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Cross-platform presence is determining presence updates based on room changes."));
    auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();
    for (auto &LocalUserStateKV : this->LocalUserStates)
    {
        auto &LocalUserState = LocalUserStateKV.Value;

        this->UpdateDesiredPresenceAdvertised(LocalUserState);

        // Iterate through rooms and collect relevant room IDs.
        TOptional<FPresenceUserAdvertisedRoom> AdvertisedSession, AdvertisedParty;
        auto ProcessRoom = [&](const FRoomRef &Room) {
            auto PresenceFeature = Room->GetRoomFeature<IPresenceRoomFeature>();
            auto PartyFeature = Room->GetRoomFeature<IPartyConfigurationRoomFeature>();
            if (PresenceFeature.IsValid())
            {
                if (PartyFeature.IsValid())
                {
                    AdvertisedParty = this->CreateAdvertisedRoomFromRoom(Room);
                }
                else
                {
                    AdvertisedSession = this->CreateAdvertisedRoomFromRoom(Room);
                }
            }
        };
        for (const auto &Room : RoomSystem->GetCurrentRooms(FGetCurrentRoomsRequest(ERoomScope::Global, NAME_Game)))
        {
            ProcessRoom(Room);
        }
        for (const auto &Room : RoomSystem->GetCurrentRooms(
                 FGetCurrentRoomsRequest(ERoomScope::User, LocalUserState->LocalUserId, NAME_Game)))
        {
            ProcessRoom(Room);
        }

        // @hack: If the room system does not yet implement sessions, call FCopyActiveSessionHandle for "GameSession" to
        // try and get information about the game session the player is currently in. This is a workaround to fill in
        // FOnlineUserPresence with the session ID until the room system refactor is complete.
        if (!this->PlatformHandle->GetSystem<IRoomSystem>()->HasRoomProvider(FName(TEXT("Session"))))
        {
            FString SessionName = TEXT("GameSession");

            EOS_EResult ResultCode;
            auto ActiveSession = FCopyActiveSessionHandle::Execute(
                this->PlatformHandle,
                FCopyActiveSessionHandle::Options{SessionName},
                ResultCode);
            if (ResultCode == EOS_EResult::EOS_Success)
            {
                FCopyInfo::Result ActiveSessionInfo;
                FCopyInfo::Execute(ActiveSession, FCopyInfo::Options{}, ResultCode, ActiveSessionInfo);
                if (ResultCode == EOS_EResult::EOS_Success)
                {
                    auto AdvertisedRoom = FPresenceUserAdvertisedRoom(
                        MakeShared<FRoomId>(FName(TEXT("Session")), ActiveSessionInfo.SessionDetails.SessionId),
                        FName(TEXT("Synthetic")));
                    AdvertisedRoom.CurrentMemberCount = ActiveSessionInfo.SessionDetails.NumPublicConnections -
                                                        ActiveSessionInfo.SessionDetails.NumOpenPublicConnections;
                    AdvertisedRoom.MaximumMemberCount = ActiveSessionInfo.SessionDetails.NumPublicConnections;
                    AdvertisedRoom.bJoinable = ActiveSessionInfo.SessionDetails.PermissionLevel ==
                                               EOS_EOnlineSessionPermissionLevel::EOS_OSPF_PublicAdvertised;
                    AdvertisedRoom.SecretPassword = TEXT("");
                    AdvertisedRoom.bRequiresSecretPassword = false;
                    AdvertisedRoom.bIsMatch = true;
                    AdvertisedRoom.bMatchStarted =
                        ActiveSessionInfo.State == EOS_EOnlineSessionState::EOS_OSS_Starting ||
                        ActiveSessionInfo.State == EOS_EOnlineSessionState::EOS_OSS_InProgress ||
                        ActiveSessionInfo.State == EOS_EOnlineSessionState::EOS_OSS_Ending;
                    AdvertisedSession = AdvertisedRoom;
                }
            }
        }

        // Assign advertised session and party IDs to desired presence state.
        if (GetTypeHash(LocalUserState->DesiredPresence.AdvertisedSession) != GetTypeHash(AdvertisedSession))
        {
            UE_LOG(
                LogRedpointEOSPresence,
                Verbose,
                TEXT("Local user %s is now advertising session room ID: %s"),
                *GetUserIdString(LocalUserState->LocalUserId),
                AdvertisedSession.IsSet() ? *AdvertisedSession->RoomId->ToString() : TEXT("(none)"));
        }
        LocalUserState->DesiredPresence.AdvertisedSession = AdvertisedSession;
        if (GetTypeHash(LocalUserState->DesiredPresence.AdvertisedParty) != GetTypeHash(AdvertisedParty))
        {
            UE_LOG(
                LogRedpointEOSPresence,
                Verbose,
                TEXT("Local user %s is now advertising party room ID: %s"),
                *GetUserIdString(LocalUserState->LocalUserId),
                AdvertisedParty.IsSet() ? *AdvertisedParty->RoomId->ToString() : TEXT("(none)"));
        }
        LocalUserState->DesiredPresence.AdvertisedParty = AdvertisedParty;
    }

    // Immediately schedule updates if room changes made the current presence out-of-date.
    this->ScheduleDesiredPresenceUpdatesWhereOutOfDate();
}

void FPresenceSystemImpl::ScheduleRemotePresenceRefreshForLocalUsers()
{
    if (this->PendingRemoteRefreshCallbacks > 0)
    {
        // Do not schedule more remote presence updates if we're still waiting for some to complete.
        return;
    }

    for (auto &LocalUserStateKV : this->LocalUserStates)
    {
        this->SyncQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSP(
            this,
            &FPresenceSystemImpl::OnProcessRequestedRemotePresenceRefresh,
            LocalUserStateKV.Key));
        this->PendingRemoteRefreshCallbacks++;
    }
}

void FPresenceSystemImpl::ScheduleRemotePresenceRefreshForLocalUser(const FIdentityUserId &LocalUserId)
{
    this->SyncQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSP(
        this,
        &FPresenceSystemImpl::OnProcessRequestedRemotePresenceRefresh,
        LocalUserId));
    this->PendingRemoteRefreshCallbacks++;
}

void FPresenceSystemImpl::OnProcessRequestedRemotePresenceRefresh(
    const FExclusiveAsyncOperationQueue::FOnDone &InOnDone,
    FIdentityUserId InLocalUserId)
{
    // Check that this local user is still tracked.
    auto LocalUserState = this->LocalUserStates.FindRef(InLocalUserId);
    if (!LocalUserState.IsValid())
    {
        // This local user is no longer registered with presence.
        this->DispatchCompletedDeferredInterestCallbacks();
        this->PendingRemoteRefreshCallbacks--;
        InOnDone.ExecuteIfBound();
        return;
    }

    // For any remote users where we haven't got their UserInfo yet, do that now so we can try to populate basic
    // presence information very quickly from the runtime platform integration.
    TArray<UE::Online::FAccountId> MissingUserInfos;
    for (const auto &KV : LocalUserState->RemoteUsers)
    {
        if (!KV.Value.UserInfo.IsValid())
        {
            MissingUserInfos.AddUnique(KV.Key);
        }
    }
    if (MissingUserInfos.Num() == 0)
    {
        this->OnProcessRequestedRemotePresenceRefreshAfterUserInfoPopulated(InOnDone, InLocalUserId);
        return;
    }

    auto UserCache = this->PlatformHandle->GetSystem<IUserCacheSystem>();
    UserCache->GetUsers(
        InLocalUserId,
        MissingUserInfos,
        IUserCacheSystem::FOnGetUsersComplete::CreateSPLambda(
            this,
            [this, InOnDone, InLocalUserId, LocalUserState](
                const FError &CallErrorCode,
                const TMap<UE::Online::FAccountId, FErrorableUserInfo> &UserInfos) {
                if (!CallErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogRedpointEOSPresence,
                        Warning,
                        TEXT("Unable to fetch user infos for interested presence users: %s"),
                        *CallErrorCode.ToLogString());
                    this->OnProcessRequestedRemotePresenceRefreshAfterUserInfoPopulated(InOnDone, InLocalUserId);
                    return;
                }

                for (const auto &KV : UserInfos)
                {
                    if (KV.Value.IsErrored())
                    {
                        UE_LOG(
                            LogRedpointEOSPresence,
                            Warning,
                            TEXT("Unable to fetch user info for interested presence user '%s': %s"),
                            *GetUserIdString(KV.Key),
                            *CallErrorCode.ToLogString());
                    }
                    else
                    {
                        auto *RemoteUserState = LocalUserState->RemoteUsers.Find(KV.Key);
                        if (RemoteUserState != nullptr)
                        {
                            RemoteUserState->UserInfo = KV.Value.GetUserInfo();
                            for (const auto &ExternalKV : RemoteUserState->UserInfo->GetLinkedExternalAccounts())
                            {
                                LocalUserState->ExternalAccountIdsToRemoteUserIds.Add(
                                    TTuple<EOS_EExternalAccountType, FString>(
                                        ExternalKV.Key,
                                        ExternalKV.Value.AccountId),
                                    KV.Key);
                            }
                        }
                    }
                }

                this->OnProcessRequestedRemotePresenceRefreshAfterUserInfoPopulated(InOnDone, InLocalUserId);
            }));
}

void FPresenceSystemImpl::OnProcessRequestedRemotePresenceRefreshAfterUserInfoPopulated(
    const FExclusiveAsyncOperationQueue::FOnDone &InOnDone,
    FIdentityUserId InLocalUserId)
{
    // Check that this local user is still tracked.
    auto LocalUserState = this->LocalUserStates.FindRef(InLocalUserId);
    if (!LocalUserState.IsValid())
    {
        // This local user is no longer registered with presence.
        this->DispatchCompletedDeferredInterestCallbacks();
        this->PendingRemoteRefreshCallbacks--;
        InOnDone.ExecuteIfBound();
        return;
    }

    // For any remote users that have a user info but no non-unified friend presence information yet, do an initial fill
    // for them.
    const auto &Integrations =
        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations();
    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, LocalUserState->LocalUserSlot);
    for (auto &KV : LocalUserState->RemoteUsers)
    {
        if (KV.Value.PartialPresence.IsSet() || !KV.Value.UserInfo.IsValid())
        {
            continue;
        }

        for (const auto &Integration : Integrations)
        {
            for (const auto &ExternalKV : KV.Value.UserInfo->GetLinkedExternalAccounts())
            {
                auto ActivityState = Integration->GetPresenceService()->GetNonUnifiedFriendPresence(
                    CallContext,
                    ExternalKV.Key,
                    ExternalKV.Value.AccountId);
                if (ActivityState.IsSet())
                {
                    // We don't store "Offline" in this dictionary, because it doesn't help us get to a point of
                    // showing the user as "not offline". If the runtime platform integrations can only provide us
                    // with an "Offline" status, then we might as well wait until the full presence information is
                    // searched from lobbies.
                    if (ActivityState.GetValue() != EPresenceUserActivityState::Offline)
                    {
                        KV.Value.PartialRuntimeActivityState.Add(
                            Integration->GetPresenceService(),
                            ActivityState.GetValue());
                    }
                }
            }
        }
        KV.Value.RecomputePartialPresence();

        // If this user now has some basic activity state from the runtime integration and no cached presence yet,
        // fire the event announcing the basic presence state.
        if (!KV.Value.IsBestPresenceFullPresence())
        {
            for (const auto &InterestedSystem : KV.Value.InterestedSystems)
            {
                UE_LOG(
                    LogRedpointEOSPresence,
                    VeryVerbose,
                    TEXT("Broadcasting OnRemoteUserPresenceStateUpdated event for interested system '%s' for local "
                         "user ID '%s' and remote presence user ID '%s', with basic activity state from runtime "
                         "platform integrations."),
                    *InterestedSystem.ToString(),
                    *GetUserIdString(InLocalUserId),
                    *GetUserIdString(KV.Key));
                this->OnRemoteUserPresenceStateUpdated(InterestedSystem)
                    .Broadcast(InLocalUserId, KV.Key, *KV.Value.GetBestPresence());
            }
        }
    }

    // Sort remote users so that iterating through the map will give us the highest priority remote users to fetch.
    LocalUserState->RemoteUsers.ValueStableSort(
        [](const FPresenceSystemRemoteUserState &A, const FPresenceSystemRemoteUserState &B) -> bool {
            if (A.FullPresence.IsSet() != B.FullPresence.IsSet())
            {
                // If they differ in whether they have initial presence, prioritize entries that have no presence
                // cached.
                return !A.FullPresence.IsSet();
            }
            else if (!A.FullPresence.IsSet())
            {
                // Neither has presence cached, so prioritize based on which one has more interested systems.
                return A.InterestedSystems.Num() >= B.InterestedSystems.Num();
            }
            else
            {
                // Both have presence cached, so prioritize those remote users that have older remote presence.
                return A.DateLastPresenceCached < B.DateLastPresenceCached;
            }
        });

    // Iterate through the map and add user IDs to query, skipping over any users that are updated too recently.
    TSet<FString> UserIdsToQuery;
    TSet<FIdentityUserId> AccountIds;
    FDateTime UtcNow = FDateTime::UtcNow();
    for (TMap<FIdentityUserId, FPresenceSystemRemoteUserState>::TConstIterator It(LocalUserState->RemoteUsers); It;
         ++It)
    {
        // Skip any remote users where the presence information was updated recently.
        if (It->Value.FullPresence.IsSet() &&
            (UtcNow - It->Value.DateLastPresenceCached).GetTotalSeconds() < MinimumAgeForRemoteUserRefresh)
        {
            continue;
        }

        // Add the user to the list to query.
        UserIdsToQuery.Add(GetUserIdString(It->Key));
        AccountIds.Add(It->Key);

        // If we have more user IDs than we can send in a query, we're done.
        int32 CurrentNumberOfUserIdsToQuery = UserIdsToQuery.Num();
        int32 NumberOfUserIdsToQueryIfWeAddedAnother = CurrentNumberOfUserIdsToQuery + 1;
        int32 NextAttributeValueFilterLength =
            (NumberOfUserIdsToQueryIfWeAddedAnother * (EOS_PRODUCTUSERID_MAX_LENGTH + 1 /* ; separator */)) -
            1 /* one less ; separator than number of user IDs */;
        if (NextAttributeValueFilterLength >= MaximumAttributeValueFilterLength)
        {
            break;
        }

        // If we would have more user IDs that we can get search results for, then we're done.
        if (NumberOfUserIdsToQueryIfWeAddedAnother > EOS_LOBBY_MAX_SEARCH_RESULTS)
        {
            break;
        }

        // If we would have more user IDs than permitted by our limit, then we're done.
        if (NumberOfUserIdsToQueryIfWeAddedAnother > MaximumProductUserIdsPerRefresh)
        {
            break;
        }
    }

    // If we don't have any remote users to query, we're done.
    if (UserIdsToQuery.Num() == 0)
    {
        this->DispatchCompletedDeferredInterestCallbacks();
        this->PendingRemoteRefreshCallbacks--;
        InOnDone.ExecuteIfBound();
        return;
    }

    // Construct the search request. Turn off durability, since we won't have results for offline users.
    auto SearchRequest = FSearchRoomsRequest(
        InLocalUserId,
        LobbyProvider,
        CrossPlatformPresenceNamespace,
        TMap<FString, FRoomAttributeConstraint>{
            {TEXT("UserId"),
             FRoomAttributeConstraint(SetToArray(UserIdsToQuery), FRoomAttributeConstraint::EConstraintType::AnyOf)},
            {TEXT("IsCrossPlatformPresence"),
             FRoomAttributeConstraint(true, FRoomAttributeConstraint::EConstraintType::Equal)}});
    // @todo: Expose a way for bucket constraints to be added to search requests.
    SearchRequest.bDurable = GIsAutomationTesting;

    // Search for the target presence lobbies.
    auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();
    RoomSystem->SearchRooms(
        SearchRequest,
        IRoomSystem::FOnSearchRoomsComplete::CreateSP(
            this,
            &FPresenceSystemImpl::OnRequestedRoomPresenceRoomsSearched,
            AccountIds,
            InOnDone,
            InLocalUserId));
}

void FPresenceSystemImpl::OnRequestedRoomPresenceRoomsSearched(
    const FError &ErrorCode,
    const TArray<FRoomSearchResultRef> &RoomSearchResults,
    TSet<FIdentityUserId> AccountIdsQueried,
    FExclusiveAsyncOperationQueue::FOnDone InOnDone,
    FIdentityUserId InLocalUserId)
{
    // If the search failed, log the error.
    if (!ErrorCode.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSPresence,
            Error,
            TEXT("Unable to query for remote user presence; EOS SDK returned error from search: %s"),
            *ErrorCode.ToLogString());
        this->PendingRemoteRefreshCallbacks--;
        InOnDone.ExecuteIfBound();
        return;
    }

    // Check that this local user is still tracked.
    auto LocalUserState = this->LocalUserStates.FindRef(InLocalUserId);
    if (!LocalUserState.IsValid())
    {
        // This local user is no longer registered with presence.
        this->DispatchCompletedDeferredInterestCallbacks();
        this->PendingRemoteRefreshCallbacks--;
        InOnDone.ExecuteIfBound();
        return;
    }

    // Create a map of the search results to the users that they represent.
    TMap<FIdentityUserId, FRoomSearchResultPtr> RoomSearchResultsByAccountId;
    for (const auto &Result : RoomSearchResults)
    {
        auto *UserId = Result->GetRoomAttributes().Find(TEXT("UserId"));
        if (UserId == nullptr)
        {
            // Unknown room search result; we should have a UserId attribute.
            UE_LOG(
                LogRedpointEOSPresence,
                Warning,
                TEXT("Ignoring room search result as it does not have a UserId attribute, even though we filtered on "
                     "it!"));
            continue;
        }

        auto AccountId = TryParseAccountId(UserId->GetString());
        if (!AccountId.IsSet())
        {
            // Unable to parse the UserId parameter back into an account ID.
            UE_LOG(
                LogRedpointEOSPresence,
                Warning,
                TEXT("Ignoring room search result as the value of the UserId attribute could not be parsed back into "
                     "an account ID."));
            continue;
        }

        RoomSearchResultsByAccountId.Add(AccountId.GetValue(), Result);
    }

    // Go through all the users that we queried for. For any user that we don't have a room search result for, that user
    // is offline.
    for (const auto &AccountId : AccountIdsQueried)
    {
        auto *RemoteTargetUserState = LocalUserState->RemoteUsers.Find(AccountId);
        if (RemoteTargetUserState == nullptr)
        {
            // The target user is no longer of interest.
            continue;
        }

        auto RoomSearchResult = RoomSearchResultsByAccountId.FindRef(AccountId);
        auto PreviousBestPresence = RemoteTargetUserState->GetBestPresence();
        if (!RoomSearchResult.IsValid())
        {
            // The target user is offline.
            RemoteTargetUserState->FullPresence = FPresenceUserState(AccountId);
            RemoteTargetUserState->FullPresence->bIsPlayingThisGame = false;
            RemoteTargetUserState->FullPresence->ActivityState = EPresenceUserActivityState::Offline;
            RemoteTargetUserState->DateLastPresenceCached = FDateTime::UtcNow();
        }
        else
        {
            RemoteTargetUserState->FullPresence =
                FPresenceUserStateSerialization::Deserialize(AccountId, RoomSearchResult.ToSharedRef());
            RemoteTargetUserState->DateLastPresenceCached = FDateTime::UtcNow();
        }

        if (GetTypeHash(PreviousBestPresence) != GetTypeHash(RemoteTargetUserState->GetBestPresence()))
        {
            for (const auto &InterestedSystem : RemoteTargetUserState->InterestedSystems)
            {
                UE_LOG(
                    LogRedpointEOSPresence,
                    VeryVerbose,
                    TEXT("Broadcasting OnRemoteUserPresenceStateUpdated event for interested system '%s' for local "
                         "user ID '%s' and remote presence user ID '%s'."),
                    *InterestedSystem.ToString(),
                    *GetUserIdString(InLocalUserId),
                    *GetUserIdString(AccountId));
                this->OnRemoteUserPresenceStateUpdated(InterestedSystem)
                    .Broadcast(InLocalUserId, AccountId, *RemoteTargetUserState->FullPresence);
            }
        }
    }

    // Dispatch any deferred calls and we're done.
    this->DispatchCompletedDeferredInterestCallbacks();
    this->PendingRemoteRefreshCallbacks--;
    InOnDone.ExecuteIfBound();
}

void FPresenceSystemImpl::DispatchCompletedDeferredInterestCallbacks()
{
    for (int i = this->DeferredInterestCallbacks.Num() - 1; i >= 0; i--)
    {
        auto &Callback = this->DeferredInterestCallbacks[i];

        // Check to see if the local user is still registered with cross-platform presence.
        auto LocalUserState = this->LocalUserStates.FindRef(Callback.LocalUserId);
        if (!LocalUserState.IsValid())
        {
            Callback.CompletionDelegate.ExecuteIfBound(
                Errors::NoConnection(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("The local user is no longer tracked in cross-platform presence before the initial "
                         "presence data of interested users could be fetched.")),
                TMap<FIdentityUserId, FPresenceUserState>());
            this->DeferredInterestCallbacks.RemoveAt(i);
            continue;
        }

        // Check to see if the local user has presence information for all the target user IDs.
        bool bHasPresenceForAllUserIds = true;
        bool bCallbackMustFailDueToInterestChange = false;
        TMap<FIdentityUserId, FPresenceUserState> InitialPresenceResults;
        for (const auto &TargetUserId : Callback.WaitingTargetUserIds)
        {
            auto *TargetUserState = LocalUserState->RemoteUsers.Find(TargetUserId);
            if (TargetUserState == nullptr)
            {
                // The target user this callback is waiting on will no longer be populated, so
                // the callback must fail.
                bCallbackMustFailDueToInterestChange = true;
                break;
            }
            else if (!TargetUserState->FullPresence.IsSet())
            {
                // We don't have full presence for one of the target users.
                bHasPresenceForAllUserIds = false;
                break;
            }
            else
            {
                // Store the cached presence so we can return this map if all target users are ready.
                InitialPresenceResults.Add(TargetUserId, *TargetUserState->GetBestPresence());
            }
        }

        // Return an error if the callback can no longer ever succeed.
        if (bCallbackMustFailDueToInterestChange)
        {
            Callback.CompletionDelegate.ExecuteIfBound(
                Errors::NoConnection(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("One or more of the remote users that this call initially expressed interest in is no longer "
                         "scheduled for an initial fetch.")),
                TMap<FIdentityUserId, FPresenceUserState>());
            this->DeferredInterestCallbacks.RemoveAt(i);
            continue;
        }

        // Fire the callback successfully and remove it if we have presence data for everyone.
        if (bHasPresenceForAllUserIds)
        {
            Callback.CompletionDelegate.ExecuteIfBound(Errors::Success(), InitialPresenceResults);
            this->DeferredInterestCallbacks.RemoveAt(i);
            continue;
        }
    }
}

IPresenceSystem::FOnInboundMessageConnectionEstablished &FPresenceSystemImpl::OnInboundMessageConnectionEstablished(
    const FName &InterestedSystem)
{
    return this->InboundMessageConnectionEstablishedDelegates.FindOrAdd(InterestedSystem);
}

void FPresenceSystemImpl::TryOpenMessageConnection(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &TargetUserId,
    const FName &InterestedSystem,
    const FOnTryOpenMessageConnection &CompletionDelegate)
{
    // Check that this local user is still tracked.
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    if (!LocalUserState.IsValid())
    {
        // This local user is no longer registered with presence.
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local user is no longer tracked in cross-platform presence.")),
            nullptr);
        return;
    }

    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Attempting to open presence message connection from local user '%s' to remote user '%s'..."),
        *GetUserIdString(LocalUserId),
        *GetUserIdString(TargetUserId));

    // Check to see if this local user already has a presence messaging connection open for this system.
    if (LocalUserState->OpenConnections.Contains(TargetUserId))
    {
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("An existing presence message connection from local user '%s' to remote user '%s' already exists. "
                 "Requesting that it create a new instance of itself..."),
            *GetUserIdString(LocalUserId),
            *GetUserIdString(TargetUserId));
        CompletionDelegate.ExecuteIfBound(
            Errors::Success(),
            LocalUserState->OpenConnections[TargetUserId]->CreateInstanced(InterestedSystem));
        return;
    }

    // Perform a durable search for the target user. We want durability here (as opposed to normal presence queries)
    // because we're explicitly trying to reach a target user and we need to join the room in order to proceed with the
    // operation.
    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Searching for rooms advertised by remote user '%s'..."),
        *GetUserIdString(TargetUserId));
    auto SearchRequest = FSearchRoomsRequest(
        LocalUserId,
        LobbyProvider,
        CrossPlatformPresenceNamespace,
        TMap<FString, FRoomAttributeConstraint>{
            {TEXT("UserId"),
             FRoomAttributeConstraint(GetUserIdString(TargetUserId), FRoomAttributeConstraint::EConstraintType::Equal)},
            {TEXT("IsCrossPlatformPresence"),
             FRoomAttributeConstraint(true, FRoomAttributeConstraint::EConstraintType::Equal)}});
    SearchRequest.bDurable = true;
    auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();
    RoomSystem->SearchRooms(
        SearchRequest,
        IRoomSystem::FOnSearchRoomsComplete::CreateSP(
            this,
            &FPresenceSystemImpl::OnSearchResultsForOpenMessageConnection,
            LocalUserId,
            TargetUserId,
            InterestedSystem,
            RoomSystem,
            CompletionDelegate));
}

void FPresenceSystemImpl::OnRemoteConnectionReceived(
    const TSharedRef<IPresenceMessageConnection> &NewConnection,
    TSharedRef<FPresenceSystemLocalUserState> LocalUserState)
{
    auto *AllEvent = this->InboundMessageConnectionEstablishedDelegates.Find(NAME_All);
    auto *SpecificEvent = this->InboundMessageConnectionEstablishedDelegates.Find(NewConnection->GetInterestedSystem());
    if (AllEvent != nullptr)
    {
        AllEvent->Broadcast(NewConnection);
    }
    if (SpecificEvent != nullptr)
    {
        SpecificEvent->Broadcast(NewConnection);
    }
}

TSharedRef<IPresenceMessageConnection> FPresenceSystemImpl::CreateAndTrackGlobalConnection(
    const TSharedRef<IRoomSystem> &RoomSystem,
    const FName &InterestedSystem,
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &TargetUserId,
    const FRoomRef &Room)
{
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    checkf(
        LocalUserState.IsValid(),
        TEXT("Local user must be tracked in presence to call CreateAndTrackMessageConnection!"));
    checkf(
        !LocalUserState->OpenConnections.Contains(TargetUserId),
        TEXT("Expected no presence message connection to already exist!"));
    checkf(!InterestedSystem.IsNone(), TEXT("Interested system must not be NAME_None!"));

    auto PresenceConnection = MakeSharedWithEvents<FPresenceMessageGlobalConnection>(
        RoomSystem,
        LocalUserId,
        TargetUserId,
        Room,
        InterestedSystem);
    PresenceConnection->OnConnectionClosedDelegate.Add(
        FPresenceMessageGlobalConnection::FOnConnectionClosed::FDelegate::CreateSPLambda(
            this,
            [TargetUserId,
             PresenceConnectionWk = TWeakPtr<FPresenceMessageGlobalConnection>(PresenceConnection),
             LocalUserStateWk = TWeakPtr<FPresenceSystemLocalUserState>(LocalUserState)]() {
                auto PresenceConnectionPinned = PresenceConnectionWk.Pin();
                auto LocalUserStatePinned = LocalUserStateWk.Pin();
                if (PresenceConnectionPinned.IsValid() && LocalUserStatePinned.IsValid())
                {
                    auto *CurrentValue = LocalUserStatePinned->OpenConnections.Find(TargetUserId);
                    if (CurrentValue != nullptr && *CurrentValue == PresenceConnectionPinned)
                    {
                        LocalUserStatePinned->OpenConnections.Remove(TargetUserId);
                    }
                }
            }));
    LocalUserState->OpenConnections.Add(TargetUserId, PresenceConnection);
    return PresenceConnection->InstancedConnections.begin()->Value.ToSharedRef();
}

void FPresenceSystemImpl::OnSearchResultsForOpenMessageConnection(
    const FError &SearchErrorCode,
    const TArray<FRoomSearchResultRef> &RoomSearchResults,
    FIdentityUserId LocalUserId,
    FIdentityUserId TargetUserId,
    FName InterestedSystem,
    TSharedRef<IRoomSystem> RoomSystem,
    FOnTryOpenMessageConnection CompletionDelegate)
{
    // Check that the search was successful.
    if (!SearchErrorCode.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSPresence,
            Error,
            TEXT("The search for rooms advertised by remote user '%s' failed: %s"),
            *GetUserIdString(TargetUserId),
            *SearchErrorCode.ToLogString());
        CompletionDelegate.ExecuteIfBound(SearchErrorCode, nullptr);
        return;
    }

    // Check that we got a search result.
    if (RoomSearchResults.Num() == 0)
    {
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("The remote user '%s' is not currently advertising any rooms."),
            *GetUserIdString(TargetUserId),
            *SearchErrorCode.ToLogString());
        CompletionDelegate.ExecuteIfBound(
            Errors::NotFound(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The target user is not currently online.")),
            nullptr);
        return;
    }

    // Check that this local user is still tracked.
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    if (!LocalUserState.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local user is no longer tracked in cross-platform presence.")),
            nullptr);
        return;
    }

    // Check to see if this local user opened multiple presence connections in parallel.
    if (LocalUserState->OpenConnections.Contains(TargetUserId))
    {
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Another presence message connection from local user '%s' to remote user '%s' was opened during the "
                 "search. Requesting that it create a new instance of itself instead..."),
            *GetUserIdString(LocalUserId),
            *GetUserIdString(TargetUserId));
        CompletionDelegate.ExecuteIfBound(
            Errors::Success(),
            LocalUserState->OpenConnections[TargetUserId]->CreateInstanced(InterestedSystem));
        return;
    }

    // We found a room belonging to the target user. Check to see if we're already in it (since multiple
    // interested systems might connect to the same target user).
    FRoomPtr ExistingRoom;
    auto CurrentRooms = RoomSystem->GetCurrentRooms(
        FGetCurrentRoomsRequest(ERoomScope::User, LocalUserId, CrossPlatformPresenceNamespace));
    for (const auto &CurrentRoom : CurrentRooms)
    {
        if (CurrentRoom->GetRoomId()->IsEqual(RoomSearchResults[0]->GetRoomId()))
        {
            ExistingRoom = CurrentRoom;
            break;
        }
    }

    // If we're already in the room, just return a new presence messaging connection immediately.
    if (ExistingRoom.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("We're already in the target room, creating a global presence connection to track our existing "
                 "membership in the room..."));
        CompletionDelegate.ExecuteIfBound(
            Errors::Success(),
            this->CreateAndTrackGlobalConnection(
                RoomSystem,
                InterestedSystem,
                LocalUserId,
                TargetUserId,
                ExistingRoom.ToSharedRef()));
        return;
    }

    // We're not in the room, so we have to join it.
    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Attempting to join advertised room for presence messaging connection..."));
    auto JoinRequest = FJoinRoomRequest(ERoomScope::User, LocalUserId, RoomSearchResults[0]);
    RoomSystem->JoinRoom(
        JoinRequest,
        IRoomSystem::FOnJoinRoomComplete::CreateSP(
            this,
            &FPresenceSystemImpl::OnJoinRoomForOpenMessageConnection,
            LocalUserId,
            TargetUserId,
            InterestedSystem,
            RoomSystem,
            CompletionDelegate));
}

void FPresenceSystemImpl::OnJoinRoomForOpenMessageConnection(
    const FError &JoinErrorCode,
    const FRoomPtr &JoinedRoom,
    FIdentityUserId LocalUserId,
    FIdentityUserId TargetUserId,
    FName InterestedSystem,
    TSharedRef<IRoomSystem> RoomSystem,
    FOnTryOpenMessageConnection CompletionDelegate)
{
    // Check that the search was successful.
    if (!JoinErrorCode.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSPresence,
            Error,
            TEXT("The join for the room advertised by remote user '%s' failed: %s"),
            *GetUserIdString(TargetUserId),
            *JoinErrorCode.ToLogString());
        CompletionDelegate.ExecuteIfBound(JoinErrorCode, nullptr);
        return;
    }

    // Check that this local user is still tracked.
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    if (!LocalUserState.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local user is no longer tracked in cross-platform presence.")),
            nullptr);
        return;
    }

    // Check to see if this local user opened multiple presence connections in parallel that would otherwise conflict.
    if (LocalUserState->OpenConnections.Contains(TargetUserId))
    {
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Another presence message connection from local user '%s' to remote user '%s' was opened during the "
                 "join. Requesting that it create a new instance of itself instead..."),
            *GetUserIdString(LocalUserId),
            *GetUserIdString(TargetUserId));
        CompletionDelegate.ExecuteIfBound(
            Errors::Success(),
            LocalUserState->OpenConnections[TargetUserId]->CreateInstanced(InterestedSystem));
        return;
    }

    // Return the new connection.
    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Creating global presence messaging connection to track the room that we just joined."));
    CompletionDelegate.ExecuteIfBound(
        Errors::Success(),
        this->CreateAndTrackGlobalConnection(
            RoomSystem,
            InterestedSystem,
            LocalUserId,
            TargetUserId,
            JoinedRoom.ToSharedRef()));
    return;
}

TOptional<FString> FPresenceSystemImpl::GetFriendCode(const FIdentityUserId &LocalUserId)
{
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    if (!LocalUserState.IsValid())
    {
        return TOptional<FString>();
    }
    return LocalUserState->FriendCode;
}

void FPresenceSystemImpl::FindByFriendCode(
    const FIdentityUserId &LocalUserId,
    const FString &FriendCode,
    const FOnFindByFriendCodeComplete &CompletionDelegate)
{
    auto SearchRequest = FSearchRoomsRequest(
        LocalUserId,
        LobbyProvider,
        CrossPlatformPresenceNamespace,
        TMap<FString, FRoomAttributeConstraint>{
            {TEXT("FriendCode"),
             FRoomAttributeConstraint(FriendCode, FRoomAttributeConstraint::EConstraintType::Equal)},
            {TEXT("IsCrossPlatformPresence"),
             FRoomAttributeConstraint(true, FRoomAttributeConstraint::EConstraintType::Equal)}});
    SearchRequest.bDurable = true;
    auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();
    RoomSystem->SearchRooms(
        SearchRequest,
        IRoomSystem::FOnSearchRoomsComplete::CreateSP(
            this,
            &FPresenceSystemImpl::OnSearchResultsForFindByFriendCode,
            LocalUserId,
            RoomSystem,
            CompletionDelegate));
}

void FPresenceSystemImpl::OnSearchResultsForFindByFriendCode(
    const FError &SearchErrorCode,
    const TArray<FRoomSearchResultRef> &RoomSearchResults,
    FIdentityUserId LocalUserId,
    TSharedRef<IRoomSystem> RoomSystem,
    FOnFindByFriendCodeComplete CompletionDelegate)
{
    if (!SearchErrorCode.WasSuccessful())
    {
        CompletionDelegate.ExecuteIfBound(SearchErrorCode, TArray<FUserInfoRef>());
        return;
    }

    TArray<FIdentityUserId> UserIds;
    for (const auto &SearchResult : RoomSearchResults)
    {
        UserIds.Add(SearchResult->GetRoomOwner()->GetUserId());
    }

    auto UserCache = this->PlatformHandle->GetSystem<IUserCacheSystem>();
    UserCache->GetUsers(
        LocalUserId,
        UserIds,
        IUserCacheSystem::FOnGetUsersComplete::CreateSPLambda(
            this,
            [CompletionDelegate](FError CallErrorCode, TMap<UE::Online::FAccountId, FErrorableUserInfo> UserInfos) {
                if (!CallErrorCode.WasSuccessful())
                {
                    CompletionDelegate.ExecuteIfBound(CallErrorCode, TArray<FUserInfoRef>());
                    return;
                }

                TArray<FUserInfoRef> Results;
                for (const auto &UserInfo : UserInfos)
                {
                    if (!UserInfo.Value.IsErrored())
                    {
                        Results.Add(UserInfo.Value.GetUserInfo());
                    }
                }
                CompletionDelegate.ExecuteIfBound(Errors::Success(), Results);
            }));
}

IPresenceSystem::FOnInternetConnectivityChanged &FPresenceSystemImpl::OnInternetConnectivityChanged()
{
    return this->InternetConnectivityChangedDelegate;
}

void FPresenceSystemImpl::RecomputeInternetConnectivityState()
{
    bool bNewDisconnectedFromInternet = false;
    for (const auto &LocalKV : this->LocalUserStates)
    {
        if (LocalKV.Value->bDisconnectedFromPresenceRoom)
        {
            bNewDisconnectedFromInternet = true;
            break;
        }
    }
    if (bNewDisconnectedFromInternet != bDisconnectedFromInternet)
    {
        bDisconnectedFromInternet = bNewDisconnectedFromInternet;
        if (bDisconnectedFromInternet)
        {
            UE_LOG(LogRedpointEOSPresence, Verbose, TEXT("Internet connectivity status is now: Disconnected"));
        }
        else
        {
            UE_LOG(LogRedpointEOSPresence, Verbose, TEXT("Internet connectivity status is now: Connected"));
        }
        this->OnInternetConnectivityChanged().Broadcast(
            bDisconnectedFromInternet ? EInternetConnectivityStatus::Disconnected
                                      : EInternetConnectivityStatus::Connected);
    }
}

void FPresenceSystemImpl::WaitForReconciledPresence(
    const FIdentityUserId &LocalUserId,
    const FSimpleDelegate &CompletionDelegate)
{
    auto LocalUserState = this->LocalUserStates.FindRef(LocalUserId);
    if (!LocalUserState.IsValid())
    {
        // No local user state; immediately fire callback as there is nothing to wait for.
        UE_LOG(
            LogRedpointEOSPresence,
            Verbose,
            TEXT("%s: Immediately firing callback as the local user has no cross-platform presence state."),
            ANSI_TO_TCHAR(__FUNCTION__));
        CompletionDelegate.ExecuteIfBound();
        return;
    }

    if (GetTypeHash(LocalUserState->CurrentPresence) == GetTypeHash(LocalUserState->DesiredPresence))
    {
        // Advertised local user presence is up-to-date with the desired presence.
        UE_LOG(
            LogRedpointEOSPresence,
            Verbose,
            TEXT("%s: Immediately firing callback as the local user's current presence matches the desired presence."),
            ANSI_TO_TCHAR(__FUNCTION__));
        CompletionDelegate.ExecuteIfBound();
        return;
    }

    // Register the callback that will fire once our presence is reconciled.
    UE_LOG(
        LogRedpointEOSPresence,
        Verbose,
        TEXT("%s: Waiting for local user's current presence to match the desired presence..."),
        ANSI_TO_TCHAR(__FUNCTION__));
    TSharedRef<FDelegateHandle> DelegateHandlePtr = MakeShared<FDelegateHandle>();
    *DelegateHandlePtr =
        LocalUserState->OnDesiredPresenceProcessedDelegate.Add(FOnDesiredPresenceProcessed::FDelegate::CreateSPLambda(
            this,
            [DelegateHandlePtr,
             LocalUserState,
             CompletionDelegate](uint32 JustProcessedPresenceUpdate, const FError &ErrorCode) {
                if (GetTypeHash(LocalUserState->CurrentPresence) == GetTypeHash(LocalUserState->DesiredPresence))
                {
                    UE_LOG(
                        LogRedpointEOSPresence,
                        Verbose,
                        TEXT("%s: Local user's current presence has now been updated to the desired presence, firing "
                             "callback."),
                        ANSI_TO_TCHAR(__FUNCTION__));
                    CompletionDelegate.ExecuteIfBound();
                    LocalUserState->OnDesiredPresenceProcessedDelegate.Remove(*DelegateHandlePtr);
                }
            }));

    // Immediately schedule updates.
    UE_LOG(
        LogRedpointEOSPresence,
        Verbose,
        TEXT("%s: Scheduling update to desired presence immediately."),
        ANSI_TO_TCHAR(__FUNCTION__));
    this->ScheduleDesiredPresenceUpdatesWhereOutOfDate();
}

void FPresenceSystemImpl::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &Commands)
{
#if UE_ALLOW_EXEC_COMMANDS
    FAutoCompleteCommand Dump;
    Dump.Command = TEXT("Dump");
    Dump.Desc = TEXT("Dump the full state of the presence system to the logs and command output.");
    Commands.Add(Dump);
#endif
}

#if REDPOINT_EOS_UE_5_3_OR_LATER

bool FPresenceSystemImpl::Exec_Runtime(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar)
{
    if (FParse::Command(&Cmd, TEXT("Dump"), false))
    {
        auto DumpPresenceAdvertisedRoom = [&Ar](const FPresenceUserAdvertisedRoom &Room, const TCHAR *Indent) {
            Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  Room ID: %s"), Indent, *Room.RoomId->ToString());
            Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  Room namespace: %s"), Indent, *Room.RoomNamespace.ToString());
            Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  Current member count: %d"), Indent, Room.CurrentMemberCount);
            Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  Maximum member count: %d"), Indent, Room.MaximumMemberCount);
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("%s  Joinable: %s"),
                Indent,
                Room.bJoinable ? TEXT("Yes") : TEXT("No"));
            Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  Secret password: %s"), Indent, *Room.SecretPassword);
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("%s  Requires secret password: %s"),
                Indent,
                Room.bRequiresSecretPassword ? TEXT("Yes") : TEXT("No"));
            Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  Is match: %s"), Indent, Room.bIsMatch ? TEXT("Yes") : TEXT("No"));
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("%s  Match started: %s"),
                Indent,
                Room.bMatchStarted ? TEXT("Yes") : TEXT("No"));
        };
        auto DumpPresenceState = [&Ar,
                                  DumpPresenceAdvertisedRoom](const FPresenceUserState &State, const TCHAR *Indent) {
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("%sPresenced advertised: %s"),
                Indent,
                State.bPresenceAdvertised ? TEXT("Yes") : TEXT("No"));
            if (State.AdvertisedSession.IsSet())
            {
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sAdvertised session:"), Indent);
                DumpPresenceAdvertisedRoom(State.AdvertisedSession.GetValue(), Indent);
            }
            else
            {
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sAdvertised session: (none)"), Indent);
            }
            if (State.AdvertisedParty.IsSet())
            {
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sAdvertised party:"), Indent);
                DumpPresenceAdvertisedRoom(State.AdvertisedParty.GetValue(), Indent);
            }
            else
            {
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sAdvertised party: (none)"), Indent);
            }
            switch (State.ActivityState)
            {
            case EPresenceUserActivityState::Online:
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sActivity state: Online"), Indent);
                break;
            case EPresenceUserActivityState::Idle:
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sActivity state: Idle"), Indent);
                break;
            case EPresenceUserActivityState::DoNotDisturb:
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sActivity state: DoNotDisturb"), Indent);
                break;
            case EPresenceUserActivityState::Offline:
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sActivity state: Offline"), Indent);
                break;
            default:
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%sActivity state: (Unknown)"), Indent);
                break;
            }
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("%sStatus text format: %s"),
                Indent,
                *State.StatusTextFormat.GetSourceString());
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("%sStatus text format arguments: (%d arguments)"),
                Indent,
                State.StatusTextFormatArguments.Num());
            for (const auto &KV : State.StatusTextFormatArguments)
            {
                switch (KV.Value.GetType())
                {
                case EFormatArgumentType::Text:
                    Ar.Logf(
                        ELogVerbosity::Verbose,
                        TEXT("%s  '%s': %s"),
                        Indent,
                        *KV.Key,
                        *KV.Value.GetTextValue().ToString());
                    break;
                case EFormatArgumentType::Int:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': %d"), Indent, *KV.Key, KV.Value.GetIntValue());
                    break;
                case EFormatArgumentType::UInt:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': %u"), Indent, *KV.Key, KV.Value.GetUIntValue());
                    break;
                case EFormatArgumentType::Float:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': %f"), Indent, *KV.Key, KV.Value.GetFloatValue());
                    break;
                case EFormatArgumentType::Double:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': %f"), Indent, *KV.Key, KV.Value.GetDoubleValue());
                    break;
                case EFormatArgumentType::Gender:
                    switch (KV.Value.GetGenderValue())
                    {
                    case ETextGender::Feminine:
                        Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': [Gender Feminine]"), Indent, *KV.Key);
                        break;
                    case ETextGender::Masculine:
                        Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': [Gender Masculine]"), Indent, *KV.Key);
                        break;
                    case ETextGender::Neuter:
                    default:
                        Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': [Gender Neutral]"), Indent, *KV.Key);
                        break;
                    }
                    break;
                default:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': (unknown value type)"), Indent, *KV.Key);
                    break;
                }
            }
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("%sStatus text formatted: %s"),
                Indent,
                *State.GetStatusTextFormatted().ToString());
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("%s  Custom attributes: (%d attributes)"),
                Indent,
                State.CustomAttributes.Num());
            for (const auto &KV : State.CustomAttributes)
            {
                switch (KV.Value.Type())
                {
                case FRoomAttribute::EType::String:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': %s"), Indent, *KV.Key, *KV.Value.GetString());
                    break;
                case FRoomAttribute::EType::Int64:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': %lld"), Indent, *KV.Key, KV.Value.GetInt64());
                    break;
                case FRoomAttribute::EType::Double:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': %f"), Indent, *KV.Key, KV.Value.GetDouble());
                    break;
                case FRoomAttribute::EType::Bool:
                    Ar.Logf(
                        ELogVerbosity::Verbose,
                        TEXT("%s  '%s': %s"),
                        Indent,
                        *KV.Key,
                        KV.Value.GetBool() ? TEXT("True") : TEXT("False"));
                    break;
                default:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("%s  '%s': (unknown value type)"), Indent, *KV.Key);
                    break;
                }
            }
            Ar.Logf(ELogVerbosity::Verbose, TEXT("%sType hash: %u"), Indent, GetTypeHash(State));
        };

        Ar.Logf(
            ELogVerbosity::Verbose,
            TEXT("Internet connectivity status: %s"),
            this->bDisconnectedFromInternet ? TEXT("Disconnected") : TEXT("Connected"));
        Ar.Logf(ELogVerbosity::Verbose, TEXT("Tracking %d local user states."), this->LocalUserStates.Num());
        for (const auto &LocalKV : this->LocalUserStates)
        {
            Ar.Logf(ELogVerbosity::Verbose, TEXT("== Presence for local user %s =="), *GetUserIdString(LocalKV.Key));
            if (!LocalKV.Value.IsValid())
            {
                Ar.Logf(ELogVerbosity::Error, TEXT("Local user state pointer is invalid!"));
                continue;
            }
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("Operation queue: %d tasks remaining / in-progress"),
                LocalKV.Value->OperationQueue->GetInProgressAndRemainingCount());
            Ar.Logf(ELogVerbosity::Verbose, TEXT("Current presence:"));
            DumpPresenceState(LocalKV.Value->CurrentPresence, TEXT("  "));
            Ar.Logf(ELogVerbosity::Verbose, TEXT("Desired presence:"));
            DumpPresenceState(LocalKV.Value->DesiredPresence, TEXT("  "));
            if (LocalKV.Value->GetCurrentPresenceRoomId().IsValid())
            {
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("Current presence room ID: %s"),
                    *LocalKV.Value->GetCurrentPresenceRoomId()->ToString());
            }
            else
            {
                Ar.Logf(ELogVerbosity::Verbose, TEXT("Current presence room ID: (none)"));
            }
            Ar.Logf(ELogVerbosity::Verbose, TEXT("Remote users: (%d tracked users)"), LocalKV.Value->RemoteUsers.Num());
            for (const auto &RemoteKV : LocalKV.Value->RemoteUsers)
            {
                Ar.Logf(ELogVerbosity::Verbose, TEXT("  %s:"), *GetUserIdString(RemoteKV.Key));
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("    Interested systems: (%d interested systems)"),
                    RemoteKV.Value.InterestedSystems.Num());
                for (const auto &InterestedSystem : RemoteKV.Value.InterestedSystems)
                {
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("      '%s'"), *InterestedSystem.ToString());
                }
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("    Has full presence: %s"),
                    RemoteKV.Value.FullPresence.IsSet() ? TEXT("Yes") : TEXT("No"));
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("    Has partial presence: %s"),
                    RemoteKV.Value.PartialPresence.IsSet() ? TEXT("Yes") : TEXT("No"));
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("    Is best presence full presence: %s"),
                    RemoteKV.Value.IsBestPresenceFullPresence() ? TEXT("Yes") : TEXT("No"));
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("    Date last full presence cached: %s (%f seconds ago)"),
                    *RemoteKV.Value.DateLastPresenceCached.ToString(),
                    (FDateTime::UtcNow() - RemoteKV.Value.DateLastPresenceCached).GetTotalSeconds());
                if (RemoteKV.Value.FullPresence.IsSet())
                {
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("    Full presence:"));
                    DumpPresenceState(*RemoteKV.Value.FullPresence, TEXT("      "));
                }
                else
                {
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("    Full presence: (none)"));
                }
                if (RemoteKV.Value.PartialPresence.IsSet())
                {
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("    Partial presence:"));
                    DumpPresenceState(*RemoteKV.Value.PartialPresence, TEXT("      "));
                }
                else
                {
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("    Partial presence: (none)"));
                }
            }
            Ar.Logf(ELogVerbosity::Verbose, TEXT("Friend code: %s"), *LocalKV.Value->FriendCode);
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("Processed presence update: %u"),
                LocalKV.Value->ProcessedPresenceUpdate);
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("Last scheduled presence update: %u"),
                LocalKV.Value->ProcessedPresenceUpdate);
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("On desired presence processed delegate: %s"),
                LocalKV.Value->OnDesiredPresenceProcessedDelegate.IsBound() ? TEXT("Bound") : TEXT("Unbound"));
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("On remote connection received delegate: %s"),
                LocalKV.Value->OnRemoteConnectionReceivedDelegate.IsBound() ? TEXT("Bound") : TEXT("Unbound"));
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("Open connections: (%d connections)"),
                LocalKV.Value->OpenConnections.Num());
            for (const auto &ConnectionKV : LocalKV.Value->OpenConnections)
            {
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("  %s: %s"),
                    *GetUserIdString(ConnectionKV.Key),
                    ConnectionKV.Value.IsValid() ? TEXT("Ptr Valid") : TEXT("Ptr Invalid"));
            }
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("Disconnected from presence room: %s"),
                LocalKV.Value->bDisconnectedFromPresenceRoom ? TEXT("Yes") : TEXT("No"));
        }

        return true;
    }

    return false;
}

#endif

}

#endif

REDPOINT_EOS_CODE_GUARD_END()