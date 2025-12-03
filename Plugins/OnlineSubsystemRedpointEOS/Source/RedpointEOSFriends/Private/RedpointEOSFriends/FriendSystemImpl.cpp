// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/FriendSystemImpl.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/Future.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSCore/Utils/UnpackMap.h"
#include "RedpointEOSFriends/Database/DatabaseIO.h"
#include "RedpointEOSFriends/Logging.h"
#include "RedpointEOSFriends/Unification/FriendUnification.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatform/Types/RelationshipType.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(775074533, Redpoint::EOS::Friends)
{
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Friends::Database;
using namespace ::Redpoint::EOS::Friends::Unification;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::UserCache;

static const FName FriendSystemName = FName(TEXT("FriendSystem"));

static const FName FriendMessageFriendRequest = FName(TEXT("FriendRequest"));
static const FName FriendMessageFriendRequestReject = FName(TEXT("FriendRequestReject"));
static const FName FriendMessageFriendRequestAccept = FName(TEXT("FriendRequestAccept"));
static const FName FriendMessageFriendDelete = FName(TEXT("FriendDelete"));

FFriendSystemImpl::FFriendSystemImpl(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , OperationQueue(MakeShared<FExclusiveAsyncOperationQueue>())
    , LocalUserFriendStates()
    , SyncTimer(MakeShared<FPeriodicTimer>(120.0f))
    , OnInboundMessageConnectionEstablishedDelegateHandle()
    , OnRemoteUserPresenceStateUpdatedDelegateHandle()
    , FriendsInitiallyLoadedDelegate()
    , FriendsChangedDelegate()
    , FriendPresenceChangedDelegate()
    , NonUnifiedFriendPresenceChangedDelegate()
    , FriendRequestReceivedDelegate()
{
}

void FFriendSystemImpl::RegisterEvents()
{
    {
        auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
        this->OnInboundMessageConnectionEstablishedDelegateHandle =
            PresenceSystem->OnInboundMessageConnectionEstablished(FriendSystemName)
                .AddSP(this, &FFriendSystemImpl::OnPresenceInboundMessageConnectionEstablished);
        this->OnRemoteUserPresenceStateUpdatedDelegateHandle =
            PresenceSystem->OnRemoteUserPresenceStateUpdated(FriendSystemName)
                .AddSP(this, &FFriendSystemImpl::OnRemoteUserPresenceStateUpdated);

        this->SyncTimer->Start(
            FPeriodicTimerDelegate::CreateSP(this, &FFriendSystemImpl::OnPeriodicTimerQueuePendingOperations));
    }

    this->PlatformHandle->OnShutdown().Add(
        FOnPlatformInstanceEvent::FDelegate::CreateSPLambda(this, [this](FPlatformInstance &) {
            this->SyncTimer->Stop();

            auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
            PresenceSystem->OnInboundMessageConnectionEstablished(FriendSystemName)
                .Remove(this->OnInboundMessageConnectionEstablishedDelegateHandle);
            PresenceSystem->OnRemoteUserPresenceStateUpdated(FriendSystemName)
                .Remove(this->OnRemoteUserPresenceStateUpdatedDelegateHandle);
        }));
}

struct FPreemptiveLocalUserFriendState
{
public:
    TSharedPtr<FLocalUserFriendState> State;
};

TFuture<FError> FFriendSystemImpl::AddLocalUser(const FIdentityUserRef &LocalUser)
{
    auto Promise = MakeShared<TPromise<FError>>();

    auto LocalUserId = LocalUser->GetUserId();
    auto Executor = MakeShared<FAsyncTaskGraph>();

    UE_LOG(
        LogRedpointEOSFriends,
        Verbose,
        TEXT("%s: Adding local user %s."),
        ANSI_TO_TCHAR(__FUNCTION__),
        *GetUserIdString(LocalUser->GetUserId()));

    // Load the friends database from Player Data Storage and create the initial LocalUserFriendState.
    auto LocalUserFriendState = MakeShared<FPreemptiveLocalUserFriendState>();
    auto PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, LocalUserId, LocalUserFriendState](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("Loading friend database for %s."),
                    *GetUserIdString(LocalUserId));
                FDatabaseIO::LoadFriendDatabase(
                    this->PlatformHandle,
                    LocalUserId,
                    FDatabaseIO::FOnFriendDatabaseLoaded::CreateSPLambda(
                        this,
                        [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                            LocalUserId,
#endif
                            LocalUserFriendState,
                            OnTaskComplete](
                            const FError &ErrorCode,
                            const TSharedPtr<FSerializedFriendDatabase> &FriendDatabase) {
                            if (!ErrorCode.WasSuccessful())
                            {
                                if (!GIsAutomationTesting)
                                {
                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Warning,
                                        TEXT("Unable to load cross-platform friends database: %s"),
                                        *ErrorCode.ToLogString());
                                }
                                LocalUserFriendState->State = MakeShared<FLocalUserFriendState>(
                                    ErrorCode.ResultCode() != EOS_EResult::EOS_InvalidState);
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSFriends,
                                    Verbose,
                                    TEXT("Created local user friend state with loaded database for %s."),
                                    *GetUserIdString(LocalUserId));
                                LocalUserFriendState->State =
                                    MakeShared<FLocalUserFriendState>(FriendDatabase.ToSharedRef());
                            }
                            checkf(
                                LocalUserFriendState->State.IsValid(),
                                TEXT("LocalUserFriendState must contain a valid pointer!"));
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        FAsyncTaskGraph::ENoDependencies::StartImmediately);

    // Register OnRelationshipsChanged events so that we can update the user's state when the platform notifies us that
    // friends, friend requests or blocked players have changed, and register OnNonUnifiedFriendPresenceChanged so we
    // can update presence for non-unified friends when the platform notifies us that they change.
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, LocalUser, LocalUserFriendState](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("Registering runtime platform relationship service events for local user %s."),
                    *GetUserIdString(LocalUser->GetUserId()));
                checkf(
                    LocalUserFriendState->State.IsValid(),
                    TEXT("LocalUserFriendState must contain a valid pointer!"));
                auto CallContext =
                    ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, LocalUser->GetUserSlot());
                for (const auto &RuntimeIntegration :
                     FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
                {
                    LocalUserFriendState->State->OnRelationshipsChangedHandles.Add(
                        RuntimeIntegration,
                        RuntimeIntegration->GetRelationshipService()
                            ->OnRelationshipsChanged(CallContext)
                            .Add(
                                IRuntimePlatformRelationshipService::FOnRelationshipsChanged::FDelegate::CreateSPLambda(
                                    this,
                                    [this, LocalUser](
                                        const TMap<ERelationshipType, TArray<FExternalUserInfoRef>> &,
                                        const TMap<ERelationshipType, TArray<FExternalUserInfoRef>> &) {
                                        // @todo: Use the added/removed parameters more efficiently, potentially by
                                        // keeping the friend unification state and just running for the changes.
                                        this->Queue(LocalUser->GetUserId(), EOperationType::Refresh)
                                            .Next([](const FError &RefreshErrorCode) {
                                                if (!RefreshErrorCode.WasSuccessful())
                                                {
                                                    UE_LOG(
                                                        LogRedpointEOSFriends,
                                                        Warning,
                                                        TEXT("Refresh operation in response to runtime platform "
                                                             "relationships changed event failed with error: %s"),
                                                        *RefreshErrorCode.ToLogString());
                                                }
                                            });
                                    })));

                    LocalUserFriendState->State->OnNonUnifiedFriendPresenceChangedHandles.Add(
                        RuntimeIntegration,
                        RuntimeIntegration->GetPresenceService()
                            ->OnNonUnifiedFriendPresenceChanged(CallContext)
                            .Add(IRuntimePlatformPresenceService::FOnNonUnifiedFriendPresenceChanged::FDelegate::
                                     CreateSPLambda(
                                         this,
                                         [this](
                                             int LocalUserNum,
                                             const EOS_EExternalAccountType &FriendAccountType,
                                             const FString &FriendAccountId,
                                             EPresenceUserActivityState NewPresenceActivityState) {
                                             this->UpdateNonUnifiedFriendPresence(
                                                 LocalUserNum,
                                                 FriendAccountType,
                                                 FriendAccountId,
                                                 NewPresenceActivityState);
                                         })));
                }
                OnTaskComplete.ExecuteIfBound(Errors::Success());
            }),
        PreviousTask);

    // Ask for an initial refresh so that friends and blocked players are populated after login.
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, LocalUserId, LocalUserFriendState, Promise](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                this->OperationQueue
                    ->AddToQueue(
                        this,
                        [this, LocalUserId, LocalUserFriendState]() -> TFuture<FError> {
                            return this->ImmediateRefresh(LocalUserId, LocalUserFriendState->State.ToSharedRef(), true)
                                .Next(IfBoundValue(
                                    this,
                                    [this, LocalUserId, LocalUserFriendState](FError RefreshError) {
                                        // Set the friend state and finish this login hook.
                                        this->LocalUserFriendStates.Add(
                                            LocalUserId,
                                            LocalUserFriendState->State.ToSharedRef());
                                        this->OnFriendsInitiallyLoaded().Broadcast(LocalUserId);
                                        return Errors::Success();
                                    },
                                    [](FError) {
                                        return Errors::Canceled(
                                            ANSI_TO_TCHAR(__FUNCTION__),
                                            TEXT("The implementor of the call went out of scope during execution."));
                                    }));
                        })
                    .Next([Promise, OnTaskComplete](FError ErrorCode) {
                        Promise->SetValue(ErrorCode);
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    });
            }),
        PreviousTask);

    // Start the work.
    Executor->Start();

    return Promise->GetFuture();
}

TFuture<void> FFriendSystemImpl::RemoveLocalUser(const FIdentityUserRef &InLocalUser)
{
    auto LocalUserId = InLocalUser->GetUserId();
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return MakeFulfilledPromise<void>().GetFuture();
    }
    auto &State = *StatePtr;

    // Unregister runtime platform integration events.
    {
        auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, InLocalUser->GetUserSlot());
        for (const auto &RuntimeIntegration :
             FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
        {
            {
                auto *Handle = State->OnRelationshipsChangedHandles.Find(RuntimeIntegration);
                if (Handle != nullptr)
                {
                    RuntimeIntegration->GetRelationshipService()->OnRelationshipsChanged(CallContext).Remove(*Handle);
                }
            }
            {
                auto *Handle = State->OnNonUnifiedFriendPresenceChangedHandles.Find(RuntimeIntegration);
                if (Handle != nullptr)
                {
                    RuntimeIntegration->GetPresenceService()
                        ->OnNonUnifiedFriendPresenceChanged(CallContext)
                        .Remove(*Handle);
                }
            }
        }
    }

    if (!State->bDirty)
    {
        UE_LOG(
            LogRedpointEOSFriends,
            Verbose,
            TEXT("Skipping flush of cross-platform friends database for local user '%s' on logout because it does not "
                 "have any unsaved changes."),
            *GetUserIdDebugString(LocalUserId));
        this->LocalUserFriendStates.Remove(LocalUserId);
        return MakeFulfilledPromise<void>().GetFuture();
    }

    return this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
        .Next(IfBoundValue(
            this,
            [this, LocalUserId](const FError &ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogRedpointEOSFriends,
                        Error,
                        TEXT("Failed to save cross-platform database changes on logout for local user '%s' - changes "
                             "may be lost!"),
                        *GetUserIdDebugString(LocalUserId));
                }
                this->LocalUserFriendStates.Remove(LocalUserId);
            },
            [](const FError &) {
            }));
}

TFuture<FError> FFriendSystemImpl::ImmediateSave(const FIdentityUserId &LocalUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        // There is no state to flush, so this counts as a success.
        return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
    }
    auto &State = *StatePtr;

    if (!State->bDirty)
    {
        // The state is no longer dirty (which can happen when we queue multiple operations to save the same
        // database).
        return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
    }

    // We clear the dirty flag here before we start saving, because if we get additional changes after we take
    // the snapshot for writing, we want subsequent queues to actually save the changes that weren't captured in
    // this snapshot. We also restore the dirty flag if we fail to save.
    State->bDirty = false;
    auto Promise = MakeShared<TPromise<FError>>();
    FDatabaseIO::SaveFriendDatabase(
        this->PlatformHandle,
        LocalUserId,
        State->FriendDatabase,
        State->bRequireNoDatabaseFilePresent,
        FDatabaseIO::FOnFriendDatabaseSaved::CreateSPLambda(
            this,
            [this, LocalUserId, Promise](const FError &ErrorCode) {
                auto *AfterSaveStatePtr = this->LocalUserFriendStates.Find(LocalUserId);
                if (AfterSaveStatePtr == nullptr)
                {
                    // The state went away while we were saving - this should not be possible, but handle it
                    // gracefully.
                    UE_LOG(LogRedpointEOSFriends, Error, TEXT("Cross-platform friend state went away during save."));
                    Promise->SetValue(Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Cross-platform friend state went away during save.")));
                    return;
                }
                auto &AfterSaveState = *AfterSaveStatePtr;

                if (!ErrorCode.WasSuccessful())
                {
                    // Restore dirty flag.
                    AfterSaveState->bDirty = true;

                    // Propagate the error.
                    Promise->SetValue(ErrorCode);
                    return;
                }

                // We've successfully written, future requests should expect a database file.
                AfterSaveState->bRequireNoDatabaseFilePresent = false;
                Promise->SetValue(Errors::Success());
            }));
    return Promise->GetFuture();
}

template <typename T> TSet<FIdentityUserId> GetAccountIdMapKeys(const TAccountIdMap<T> &Map)
{
    TSet<FIdentityUserId> Set;
    for (const auto &KV : Map)
    {
        Set.Add(KV.Key);
    }
    return Set;
}

TFuture<FError> FFriendSystemImpl::ImmediateRefresh(
    const FIdentityUserId &LocalUserId,
    const TSharedRef<FLocalUserFriendState> &LocalUserFriendState,
    bool bSilenceEvents)
{
    auto Executor = MakeShared<FAsyncTaskGraph>();

    // Perform friend unification.
    auto IncomingLocalUserFriendState = MakeShared<FIncomingLocalUserFriendState>();
    auto PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, LocalUserId, LocalUserFriendState, IncomingLocalUserFriendState](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("Querying unified friends list for '%s'..."),
                    *GetUserIdString(LocalUserId));
                FFriendUnification::ReadUnifiedFriends(
                    this->PlatformHandle,
                    LocalUserId,
                    LocalUserFriendState,
                    FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
                    FFriendUnification::FOnReadUnifiedFriendsComplete::CreateSPLambda(
                        this,
                        [IncomingLocalUserFriendState, OnTaskComplete](
                            const FError &ErrorCode,
                            const TAccountIdMap<FUnifiedUserInfoPtr> &UnifiedFriends,
                            const TArray<FExternalUserInfoRef> &NonUnifiedFriends) {
                            if (!ErrorCode.WasSuccessful())
                            {
                                UE_LOG(
                                    LogRedpointEOSFriends,
                                    Warning,
                                    TEXT("Unable to refresh friends list: %s"),
                                    *ErrorCode.ToLogString());
                            }
                            else
                            {
                                TAccountIdMap<FUserInfoRef> UnifiedFriendsAsUsers;
                                for (const auto &Friend : UnifiedFriends)
                                {
                                    UnifiedFriendsAsUsers.Add(Friend.Key, Friend.Value.ToSharedRef());
                                }
                                IncomingLocalUserFriendState->UnifiedFriends = UnifiedFriendsAsUsers;
                                IncomingLocalUserFriendState->NonUnifiedFriends = NonUnifiedFriends;
                            }
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        FAsyncTaskGraph::ENoDependencies::StartImmediately);

    // Lookup user infos for recent players.
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, LocalUserId, LocalUserFriendState, IncomingLocalUserFriendState](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("Querying recent players list for '%s'..."),
                    *GetUserIdString(LocalUserId));
                TArray<FIdentityUserId> UsersToLookup;
                for (const auto &KV : LocalUserFriendState->FriendDatabase->RecentPlayers)
                {
                    UsersToLookup.Add(KV.Key);
                }
                for (const auto &KV : LocalUserFriendState->FriendDatabase->PendingFriendRequests)
                {
                    if (KV.Value.Mode == ESerializedPendingFriendRequestMode::InboundPendingSendRejection ||
                        KV.Value.Mode == ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion)
                    {
                        // Should not appear on the friends list. Only exists in the database because we still
                        // have to send the message to the target user.
                        continue;
                    }

                    if (KV.Value.Mode == ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance)
                    {
                        // There will already be an entry from the friends list.
                        continue;
                    }

                    UsersToLookup.Add(KV.Key);
                }

                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("There are %d recent players to query for '%s'."),
                    UsersToLookup.Num(),
                    *GetUserIdString(LocalUserId));

                auto UserCacheSystem = this->PlatformHandle->GetSystem<IUserCacheSystem>();
                UserCacheSystem->GetUsers(
                    LocalUserId,
                    UsersToLookup,
                    IUserCacheSystem::FOnGetUsersComplete::CreateSPLambda(
                        this,
                        [LocalUserFriendState, IncomingLocalUserFriendState, OnTaskComplete](
                            const FError &CallErrorCode,
                            const TMap<UE::Online::FAccountId, FErrorableUserInfo> &UserInfos) {
                            TAccountIdMap<TTuple<FUserInfoRef, FDateTime>> RecentPlayers;
                            TAccountIdMap<FUserInfoRef> InboundFriendRequests;
                            TAccountIdMap<FUserInfoRef> OutboundFriendRequests;
                            for (const auto &KV : UserInfos)
                            {
                                if (!KV.Value.IsErrored())
                                {
                                    auto *RecentInfo = LocalUserFriendState->FriendDatabase->RecentPlayers.Find(KV.Key);
                                    if (RecentInfo != nullptr)
                                    {
                                        RecentPlayers.Add(
                                            KV.Key,
                                            TTuple<FUserInfoRef, FDateTime>(
                                                KV.Value.GetUserInfo(),
                                                RecentInfo->LastSeen));
                                    }

                                    auto *PendingInfo =
                                        LocalUserFriendState->FriendDatabase->PendingFriendRequests.Find(KV.Key);
                                    if (PendingInfo != nullptr)
                                    {
                                        switch (PendingInfo->Mode)
                                        {
                                        case ESerializedPendingFriendRequestMode::InboundPendingResponse:
                                            InboundFriendRequests.Add(KV.Key, KV.Value.GetUserInfo());
                                            break;
                                        case ESerializedPendingFriendRequestMode::OutboundPendingSend:
                                        case ESerializedPendingFriendRequestMode::__NoLongerUsed_OutboundPendingReceive:
                                        case ESerializedPendingFriendRequestMode::OutboundPendingResponse:
                                            OutboundFriendRequests.Add(KV.Key, KV.Value.GetUserInfo());
                                            break;
                                        default:
                                            // No action.
                                            break;
                                        }
                                    }
                                }
                            }

                            // Set the recent players, inbound friend requests and outbound friend requests
                            // in the local friend user state.
                            IncomingLocalUserFriendState->RecentPlayers = RecentPlayers;
                            IncomingLocalUserFriendState->InboundFriendRequests = InboundFriendRequests;
                            IncomingLocalUserFriendState->OutboundFriendRequests = OutboundFriendRequests;

                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        PreviousTask);

    // Aggregate blocked players from the cross-platform friends database and the runtime platform integrations.
    auto BlockedUserIds = MakeShared<TMap<FIdentityUserId, FUserInfoPtr>>();
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, LocalUserId, LocalUserFriendState, BlockedUserIds](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("Querying blocked players list from runtime platforms for '%s'..."),
                    *GetUserIdString(LocalUserId));
                for (const auto &BlockedPlayer : LocalUserFriendState->FriendDatabase->BlockedUsers)
                {
                    BlockedUserIds->Add(BlockedPlayer.Key, nullptr);
                }

                auto IdentitySystem = PlatformHandle->GetSystem<IIdentitySystem>();
                auto LocalUser = IdentitySystem->GetUser(LocalUserId);
                if (!LocalUser.IsValid())
                {
                    // User is no longer signed in.
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                    return;
                }

                auto CallContext = ConstructRuntimePlatformServiceCallContext(PlatformHandle, LocalUser->GetUserSlot());
                using FIntegrationOp = FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, bool>;
                FIntegrationOp::RunSP(
                    this->AsShared(),
                    FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
                    [this, CallContext, LocalUserId, BlockedUserIds](
                        TSharedRef<IRuntimePlatformIntegration> Integration,
                        TFunction<void(bool)> OnIntegrationDone) -> bool {
                        Integration->GetRelationshipService()->GetRelationships(
                            CallContext,
                            ERelationshipType::Blocked,
                            IRuntimePlatformRelationshipService::FOnGetRelationshipComplete::CreateSPLambda(
                                this,
                                [this, LocalUserId, BlockedUserIds, OnIntegrationDone](
                                    const FError &QueryError,
                                    const TMap<ERelationshipType, TArray<FExternalUserInfoRef>> &RelationshipsByType) {
                                    if (!QueryError.WasSuccessful())
                                    {
                                        if (!GIsAutomationTesting)
                                        {
                                            UE_LOG(
                                                LogRedpointEOSFriends,
                                                Warning,
                                                TEXT("Unable to query blocked players from runtime "
                                                     "platform integration: %s"),
                                                *QueryError.ToLogString());
                                        }
                                        OnIntegrationDone(true);
                                        return;
                                    }

                                    auto BlockedExternalUserInfos =
                                        RelationshipsByType.FindRef(ERelationshipType::Blocked);
                                    if (BlockedExternalUserInfos.Num() == 0)
                                    {
                                        OnIntegrationDone(true);
                                        return;
                                    }

                                    TMap<EOS_EExternalAccountType, TArray<FString>> ExternalAccountIdsByType;
                                    for (const auto &ExternalUserInfo : BlockedExternalUserInfos)
                                    {
                                        for (const auto &LinkedAccount : ExternalUserInfo->GetLinkedExternalAccounts())
                                        {
                                            ExternalAccountIdsByType.FindOrAdd(LinkedAccount.Key)
                                                .Add(LinkedAccount.Value.AccountId);
                                        }
                                    }

                                    // Look up these users so we have their PUIDs.
                                    using FGetOp =
                                        FMultiOperation<TTuple<EOS_EExternalAccountType, TArray<FString>>, bool>;
                                    FGetOp::RunSP(
                                        this,
                                        UnpackMap(ExternalAccountIdsByType),
                                        [this, LocalUserId, BlockedUserIds](
                                            TTuple<EOS_EExternalAccountType, TArray<FString>> Value,
                                            TFunction<void(bool)> OnGetDone) -> bool {
                                            auto UserCacheSystem = this->PlatformHandle->GetSystem<IUserCacheSystem>();
                                            UserCacheSystem->GetUsersByExternalAccountIds(
                                                LocalUserId,
                                                Value.Get<0>(),
                                                Value.Get<1>(),
                                                IUserCacheSystem::FOnGetUsersByExternalAccountIdsComplete::
                                                    CreateSPLambda(
                                                        this,
                                                        [BlockedUserIds, OnGetDone](
                                                            const FError &CallErrorCode,
                                                            const TMap<FString, FErrorableUserInfo> &UserInfos) {
                                                            for (const auto &KV : UserInfos)
                                                            {
                                                                if (!KV.Value.IsErrored() &&
                                                                    !BlockedUserIds->Contains(
                                                                        KV.Value.GetUserInfo()->GetUserId()))
                                                                {
                                                                    BlockedUserIds->Add(
                                                                        KV.Value.GetUserInfo()->GetUserId(),
                                                                        KV.Value.GetUserInfo());
                                                                }
                                                            }
                                                            OnGetDone(true);
                                                        }));
                                            return true;
                                        },
                                        [OnIntegrationDone](const TArray<bool> &) {
                                            OnIntegrationDone(true);
                                        });
                                }));
                        return true;
                    },
                    [OnTaskComplete](const TArray<bool> &) {
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    });
            }),
        PreviousTask);

    // Lookup user infos for any blocked users that we don't have yet (i.e. blocked users that came from the
    // cross-platform friend database).
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, LocalUserId, IncomingLocalUserFriendState, BlockedUserIds](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("Querying blocked players list from cross-platform friend database for '%s'..."),
                    *GetUserIdString(LocalUserId));
                TArray<FIdentityUserId> UsersToLookup;
                for (const auto &KV : *BlockedUserIds)
                {
                    if (!KV.Value.IsValid())
                    {
                        UsersToLookup.Add(KV.Key);
                    }
                }

                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("There are %d blocked users to query from the cross-platform friend database for '%s'."),
                    UsersToLookup.Num(),
                    *GetUserIdString(LocalUserId));

                auto UserCacheSystem = this->PlatformHandle->GetSystem<IUserCacheSystem>();
                UserCacheSystem->GetUsers(
                    LocalUserId,
                    UsersToLookup,
                    IUserCacheSystem::FOnGetUsersComplete::CreateSPLambda(
                        this,
                        [IncomingLocalUserFriendState, BlockedUserIds, OnTaskComplete](
                            const FError &CallErrorCode,
                            const TMap<UE::Online::FAccountId, FErrorableUserInfo> &UserInfos) {
                            for (const auto &KV : UserInfos)
                            {
                                if (!KV.Value.IsErrored())
                                {
                                    (*BlockedUserIds)[KV.Key] = KV.Value.GetUserInfo();
                                }
                            }

                            // Now update the blocked users map in the local friend user state.
                            IncomingLocalUserFriendState->BlockedPlayers = *BlockedUserIds;

                            // Forcibly remove any blocked players from unified friends, recent players and friend
                            // requests, so that blocked players never appear through any other friend APIs. This is
                            // necessary because a runtime platform might provide a friend whose PUID is blocked, and
                            // our game-level block list should take priority over platform level friends.
                            for (const auto &KV : IncomingLocalUserFriendState->BlockedPlayers)
                            {
                                IncomingLocalUserFriendState->UnifiedFriends.Remove(KV.Key);
                                IncomingLocalUserFriendState->InboundFriendRequests.Remove(KV.Key);
                                IncomingLocalUserFriendState->OutboundFriendRequests.Remove(KV.Key);
                                IncomingLocalUserFriendState->RecentPlayers.Remove(KV.Key);
                            }

                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        PreviousTask);

    // Calculate differences between current local user state and incoming local user state, update the presence
    // interest list, update the current state and fire events.
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, bSilenceEvents, LocalUserId, LocalUserFriendState, IncomingLocalUserFriendState](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                // Figure out what's changed in this update.
                auto RelationshipChanges = ERelationshipType::None;
                TArray<FIdentityUserId> NewInboundInvites;
                {
                    auto IncomingKeys = GetAccountIdMapKeys(IncomingLocalUserFriendState->UnifiedFriends);
                    auto CurrentKeys = GetAccountIdMapKeys(LocalUserFriendState->UnifiedFriends);
                    if (IncomingKeys.Num() != CurrentKeys.Num() ||
                        IncomingKeys.Union(CurrentKeys).Num() != IncomingKeys.Num())
                    {
                        RelationshipChanges |= ERelationshipType::MutualFriends;
                    }
                }
                {
                    auto GetExternalUserIdKeys = [](const TArray<FExternalUserInfoRef> &Users) {
                        TSet<TTuple<EOS_EExternalAccountType, FString>> ExternalAccountIds;
                        for (const auto &User : Users)
                        {
                            for (const auto &LinkedAccount : User->GetLinkedExternalAccounts())
                            {
                                ExternalAccountIds.Add(TTuple<EOS_EExternalAccountType, FString>(
                                    LinkedAccount.Key,
                                    LinkedAccount.Value.AccountId));
                            }
                        }
                        return ExternalAccountIds;
                    };
                    auto IncomingKeys = GetExternalUserIdKeys(IncomingLocalUserFriendState->NonUnifiedFriends);
                    auto CurrentKeys = GetExternalUserIdKeys(LocalUserFriendState->NonUnifiedFriends);
                    if (IncomingKeys.Num() != CurrentKeys.Num() ||
                        IncomingKeys.Union(CurrentKeys).Num() != IncomingKeys.Num())
                    {
                        RelationshipChanges |= ERelationshipType::MutualFriends;
                    }
                }
                {
                    auto IncomingKeys = GetAccountIdMapKeys(IncomingLocalUserFriendState->InboundFriendRequests);
                    auto CurrentKeys = GetAccountIdMapKeys(LocalUserFriendState->InboundFriendRequests);
                    if (IncomingKeys.Num() != CurrentKeys.Num() ||
                        IncomingKeys.Union(CurrentKeys).Num() != IncomingKeys.Num())
                    {
                        RelationshipChanges |= ERelationshipType::IncomingFriendRequest;
                    }

                    for (const auto &UserId : IncomingKeys.Difference(CurrentKeys))
                    {
                        NewInboundInvites.Add(UserId);
                    }
                }
                {
                    auto IncomingKeys = GetAccountIdMapKeys(IncomingLocalUserFriendState->OutboundFriendRequests);
                    auto CurrentKeys = GetAccountIdMapKeys(LocalUserFriendState->OutboundFriendRequests);
                    if (IncomingKeys.Num() != CurrentKeys.Num() ||
                        IncomingKeys.Union(CurrentKeys).Num() != IncomingKeys.Num())
                    {
                        RelationshipChanges |= ERelationshipType::OutgoingFriendRequest;
                    }
                }
                {
                    auto IncomingKeys = GetAccountIdMapKeys(IncomingLocalUserFriendState->BlockedPlayers);
                    auto CurrentKeys = GetAccountIdMapKeys(LocalUserFriendState->BlockedPlayers);
                    if (IncomingKeys.Num() != CurrentKeys.Num() ||
                        IncomingKeys.Union(CurrentKeys).Num() != IncomingKeys.Num())
                    {
                        RelationshipChanges |= ERelationshipType::Blocked;
                    }
                }
                {
                    auto IncomingKeys = GetAccountIdMapKeys(IncomingLocalUserFriendState->RecentPlayers);
                    auto CurrentKeys = GetAccountIdMapKeys(LocalUserFriendState->RecentPlayers);
                    if (IncomingKeys.Num() != CurrentKeys.Num() ||
                        IncomingKeys.Union(CurrentKeys).Num() != IncomingKeys.Num())
                    {
                        RelationshipChanges |= ERelationshipType::RecentPlayer;
                    }
                }

                // Look up user information for all incoming invites.
                auto UserCacheSystem = this->PlatformHandle->GetSystem<IUserCacheSystem>();
                UserCacheSystem->GetUsers(
                    LocalUserId,
                    NewInboundInvites,
                    IUserCacheSystem::FOnGetUsersComplete::CreateSPLambda(
                        this,
                        [this,
                         LocalUserId,
                         LocalUserFriendState,
                         bSilenceEvents,
                         IncomingLocalUserFriendState,
                         RelationshipChanges,
                         NewInboundInvites,
                         OnTaskComplete](
                            const FError &CallErrorCode,
                            const TMap<UE::Online::FAccountId, FErrorableUserInfo> &UserInfos) {
                            // Determine differences for presence interest.
                            TSet<FIdentityUserId> DesiredPresenceUserIds;
                            for (const auto &Friend : IncomingLocalUserFriendState->UnifiedFriends)
                            {
                                DesiredPresenceUserIds.Add(Friend.Key);
                            }
                            for (const auto &FriendRequest : IncomingLocalUserFriendState->InboundFriendRequests)
                            {
                                DesiredPresenceUserIds.Add(FriendRequest.Key);
                            }
                            for (const auto &FriendRequest : IncomingLocalUserFriendState->OutboundFriendRequests)
                            {
                                DesiredPresenceUserIds.Add(FriendRequest.Key);
                            }
                            auto PresenceToRemove =
                                LocalUserFriendState->PresenceInterestList.Difference(DesiredPresenceUserIds);
                            auto PresenceToAdd =
                                DesiredPresenceUserIds.Difference(LocalUserFriendState->PresenceInterestList);
                            TArray<FPresenceInterestChange> PresenceInterestChanges;
                            for (const auto &Change : PresenceToRemove)
                            {
                                PresenceInterestChanges.Add(FPresenceInterestChange(Change, false));
                            }
                            for (const auto &Change : PresenceToAdd)
                            {
                                PresenceInterestChanges.Add(FPresenceInterestChange(Change, true));
                            }
                            auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
                            PresenceSystem->UpdatePresenceInterest(
                                FriendSystemName,
                                LocalUserId,
                                PresenceInterestChanges,
                                false,
                                IPresenceSystem::FOnUpdatePresenceInterestComplete::CreateSPLambda(
                                    this,
                                    [](const FError &PresenceErrorCode,
                                       const TMap<FIdentityUserId, FPresenceUserState> &) {
                                        if (PresenceErrorCode.ResultCode() == EOS_EResult::EOS_NotFound)
                                        {
                                            UE_LOG(
                                                LogRedpointEOSFriends,
                                                Warning,
                                                TEXT("Unable to update presence interest as the local user does not "
                                                     "have cross-platform presence. This will occur if the current "
                                                     "client ID has a role that does not have the EOS lobbies "
                                                     "service enabled. The local user will not be able to see "
                                                     "cross-platform presence statuses on their friends list, which "
                                                     "may prevent them from joining games, parties or interacting with "
                                                     "invites. You should update your client ID's role/permissions in "
                                                     "the EOS Developer Portal to fix this issue. The full error "
                                                     "message was: %s"),
                                                *PresenceErrorCode.ToLogString());
                                        }
                                        else if (!PresenceErrorCode.WasSuccessful())
                                        {
                                            UE_LOG(
                                                LogRedpointEOSFriends,
                                                Error,
                                                TEXT("Unable to update presence interest of the local user due to an "
                                                     "unexpected error. The full error message was: %s"),
                                                *PresenceErrorCode.ToLogString());
                                        }
                                    }));

                            LocalUserFriendState->PresenceInterestList = DesiredPresenceUserIds;
                            for (const auto &Change : PresenceToRemove)
                            {
                                UE_LOG(
                                    LogRedpointEOSFriends,
                                    Verbose,
                                    TEXT("Removing %s from presence interest list."),
                                    *GetUserIdString(Change));
                                LocalUserFriendState->UnifiedFriendsPresenceUserState.Remove(Change);
                            }
                            for (const auto &Change : PresenceToAdd)
                            {
                                auto AvailablePresenceState = PresenceSystem->GetPresence(LocalUserId, Change);
                                if (AvailablePresenceState.IsSet())
                                {
                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Verbose,
                                        TEXT("Adding %s to presence interest list (got initial presence "
                                             "state)."),
                                        *GetUserIdString(Change));
                                    LocalUserFriendState->UnifiedFriendsPresenceUserState.Add(
                                        Change,
                                        AvailablePresenceState.GetValue());
                                }
                                else
                                {
                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Verbose,
                                        TEXT("Adding %s to presence interest list (no initial presence "
                                             "available)."),
                                        *GetUserIdString(Change));
                                }
                            }

                            auto IdentitySystem = PlatformHandle->GetSystem<IIdentitySystem>();
                            auto LocalUser = IdentitySystem->GetUser(LocalUserId);
                            if (LocalUser.IsValid())
                            {
                                auto CallContext = ConstructRuntimePlatformServiceCallContext(
                                    PlatformHandle,
                                    LocalUser->GetUserSlot());

                                // @todo: Remove old non-unified friend presence states.
                                for (const auto &NonUnifiedFriend : IncomingLocalUserFriendState->NonUnifiedFriends)
                                {
                                    if (LocalUserFriendState->NonUnifiedFriends.Contains(NonUnifiedFriend))
                                    {
                                        // We should already have presence info for this friend (or we'll
                                        // get it through an event).
                                        continue;
                                    }

                                    auto bHandled = false;
                                    for (const auto &LinkedAccount : NonUnifiedFriend->GetLinkedExternalAccounts())
                                    {
                                        for (const auto &RuntimeIntegration : FModule::GetModuleChecked()
                                                                                  .GetRuntimePlatformRegistry()
                                                                                  ->GetRuntimePlatformIntegrations())
                                        {
                                            auto PresenceInfo =
                                                RuntimeIntegration->GetPresenceService()->GetNonUnifiedFriendPresence(
                                                    CallContext,
                                                    LinkedAccount.Key,
                                                    LinkedAccount.Value.AccountId);
                                            if (PresenceInfo.IsSet())
                                            {
                                                LocalUserFriendState->NonUnifiedFriendsPresenceUserActivityState.Add(
                                                    TTuple<EOS_EExternalAccountType, FString>(
                                                        LinkedAccount.Key,
                                                        LinkedAccount.Value.AccountId),
                                                    PresenceInfo.GetValue());
                                                bHandled = true;
                                                break;
                                            }
                                        }
                                        if (bHandled)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }

                            // First update the local user friend state so that if the developer calls APIs
                            // based on events firing, they'll see the correct state.
                            LocalUserFriendState->UnifiedFriends = IncomingLocalUserFriendState->UnifiedFriends;
                            LocalUserFriendState->NonUnifiedFriends = IncomingLocalUserFriendState->NonUnifiedFriends;
                            LocalUserFriendState->BlockedPlayers = IncomingLocalUserFriendState->BlockedPlayers;
                            LocalUserFriendState->RecentPlayers = IncomingLocalUserFriendState->RecentPlayers;
                            LocalUserFriendState->InboundFriendRequests =
                                IncomingLocalUserFriendState->InboundFriendRequests;
                            LocalUserFriendState->OutboundFriendRequests =
                                IncomingLocalUserFriendState->OutboundFriendRequests;

                            if (!bSilenceEvents)
                            {
                                // Go through all the invites and fire events for the users we could find (which
                                // should be all of them).
                                if (CallErrorCode.WasSuccessful())
                                {
                                    for (const auto &NewInboundInvite : NewInboundInvites)
                                    {
                                        auto *UserInfo = UserInfos.Find(NewInboundInvite);
                                        if (UserInfo != nullptr && !UserInfo->IsErrored())
                                        {
                                            this->OnFriendRequestReceived().Broadcast(
                                                LocalUserId,
                                                UserInfo->GetUserInfo());
                                        }
                                    }
                                }

                                if (RelationshipChanges != ERelationshipType::None)
                                {
                                    this->OnFriendsChanged().Broadcast(LocalUserId, RelationshipChanges);
                                }
                            }

                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        PreviousTask);

    // We're finished.
    auto Promise = MakeShared<TPromise<FError>>();
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [Promise](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                OnTaskComplete.ExecuteIfBound(Errors::Success());
                Promise->SetValue(Errors::Success());
            }),
        PreviousTask);

    // Start the work.
    Executor->Start();
    return Promise->GetFuture();
}

TFuture<FError> FFriendSystemImpl::Immediate(const FIdentityUserId &InAccountId, EOperationType OperationType)
{
    if ((OperationType & EOperationType::Save) != EOperationType::None)
    {
        return this->ImmediateSave(InAccountId) +
               IfBoundAwait(
                   this,
                   [this, InAccountId, OperationType](FError SaveResult) {
                       if (!SaveResult.WasSuccessful())
                       {
                           return MakeFulfilledPromise<FError>(SaveResult).GetFuture();
                       }

                       if ((OperationType & EOperationType::Refresh) != EOperationType::None)
                       {
                           auto *StatePtr = this->LocalUserFriendStates.Find(InAccountId);
                           if (StatePtr == nullptr)
                           {
                               // The user is not signed in.
                               return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
                           }
                           auto &State = *StatePtr;

                           return this->ImmediateRefresh(InAccountId, State, false);
                       }
                       else
                       {
                           return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
                       }
                   },
                   [](const FError &SaveResult) {
                       return MakeFulfilledPromise<FError>(
                                  Errors::Canceled(
                                      ANSI_TO_TCHAR(__FUNCTION__),
                                      TEXT("The friends system was shutdown before this call could "
                                           "complete.")))
                           .GetFuture();
                   });
    }
    else if ((OperationType & EOperationType::Refresh) != EOperationType::None)
    {
        auto *StatePtr = this->LocalUserFriendStates.Find(InAccountId);
        if (StatePtr == nullptr)
        {
            // The user is not signed in.
            return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
        }
        auto &State = *StatePtr;

        return this->ImmediateRefresh(InAccountId, State, false);
    }
    else
    {
        return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
    }
}

TFuture<FError> FFriendSystemImpl::Queue(const FIdentityUserId &InAccountId, EOperationType OperationType)
{
    return this->OperationQueue->AddToQueue(this, [this, InAccountId, OperationType]() -> TFuture<FError> {
        return this->Immediate(InAccountId, OperationType);
    });
}

void FFriendSystemImpl::AddPendingFriendRequest(
    const TSharedRef<FLocalUserFriendState> &State,
    const FIdentityUserId &RemoteUserId,
    FSerializedPendingFriendRequest &&Request,
    const FString &Context)
{
    checkf(
        State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId) == nullptr,
        TEXT("Expected there to be no existing friend request when AddPendingFriendRequest was called: %s"),
        *Context);
    State->FriendDatabase->PendingFriendRequests.Add(RemoteUserId, Request);
}

void FFriendSystemImpl::RemovePendingFriendRequest(
    const TSharedRef<FLocalUserFriendState> &State,
    const FIdentityUserId &RemoteUserId,
    const FString &Context)
{
    auto *ExistingFriendRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
    if (ExistingFriendRequest != nullptr)
    {
        UE_LOG(
            LogRedpointEOSFriends,
            VeryVerbose,
            TEXT("Removing existing pending friend request with mode %d in context '%s'."),
            ExistingFriendRequest->Mode,
            *Context);

        State->FriendDatabase->PendingFriendRequests.Remove(RemoteUserId);
    }
}

void FFriendSystemImpl::OnPresenceInboundMessageConnectionEstablished(
    const TSharedRef<IPresenceMessageConnection> &MessageConnection)
{
    UE_LOG(
        LogRedpointEOSFriends,
        VeryVerbose,
        TEXT("Received inbound messaging connection for local user '%s' from remote user '%s'."),
        *GetUserIdString(MessageConnection->GetLocalUserId()),
        *GetUserIdString(MessageConnection->GetTargetUserId()));
    auto LocalUserId = MessageConnection->GetLocalUserId();
    auto RemoteUserId = MessageConnection->GetTargetUserId();
    MessageConnection->OnMessageReceivedOrClose().Add(
        IPresenceMessageConnection::FOnMessageReceivedOrCloseDelegate::FDelegate::CreateSPLambda(
            this,
            [this, MessageConnection, LocalUserId, RemoteUserId](
                bool bIsClosed,
                const FName &MessageType,
                const TSharedPtr<FJsonValue> &Message,
                const IPresenceMessageConnection::FAckMessage &AckMessage) {
                if (bIsClosed)
                {
                    UE_LOG(
                        LogRedpointEOSFriends,
                        VeryVerbose,
                        TEXT("Processing CLOSE for local user '%s' from remote user '%s'."),
                        *GetUserIdString(MessageConnection->GetLocalUserId()),
                        *GetUserIdString(MessageConnection->GetTargetUserId()));
                }
                else
                {
                    auto JsonWrappingObject = MakeShared<FJsonObject>();
                    JsonWrappingObject->SetField(TEXT("Content"), Message);

                    FString ContentString;
                    TSharedRef<TJsonWriter<TCHAR>> Writer = TJsonWriterFactory<TCHAR>::Create(&ContentString);
                    FJsonSerializer::Serialize(JsonWrappingObject, Writer);
                    UE_LOG(
                        LogRedpointEOSFriends,
                        VeryVerbose,
                        TEXT("Processing MESSAGE for local user '%s' from remote user '%s', with content: %s"),
                        *GetUserIdString(MessageConnection->GetLocalUserId()),
                        *GetUserIdString(MessageConnection->GetTargetUserId()),
                        *ContentString);
                }

                if (bIsClosed)
                {
                    return;
                }

                auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
                if (StatePtr == nullptr)
                {
                    UE_LOG(
                        LogRedpointEOSFriends,
                        Warning,
                        TEXT("%s"),
                        *Errors::InvalidUser(
                             ANSI_TO_TCHAR(__FUNCTION__),
                             TEXT("The local user for whom the friends message was received is not currently signed "
                                  "in. The message will be dropped."))
                             .ToLogString());
                    return;
                }
                auto &State = *StatePtr;

                if (MessageType.IsEqual(FriendMessageFriendRequest))
                {
                    auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
                    if (ExistingRequest != nullptr)
                    {
                        if (ExistingRequest->Mode == ESerializedPendingFriendRequestMode::InboundPendingResponse)
                        {
                            // Could be a resend, ack it.
                            AckMessage.ExecuteIfBound();
                        }
                        else
                        {
                            UE_LOG(
                                LogRedpointEOSFriends,
                                Warning,
                                TEXT("%s"),
                                *Errors::UnexpectedError(
                                     ANSI_TO_TCHAR(__FUNCTION__),
                                     FString::Printf(
                                         TEXT("Received a friend request from '%s', but we are already processing a "
                                              "friend request for this user. The message will be dropped."),
                                         *GetUserIdString(RemoteUserId)))
                                     .ToLogString());
                        }
                        return;
                    }

                    {
                        FSerializedPendingFriendRequest NewRequest = {};
                        NewRequest.Mode = ESerializedPendingFriendRequestMode::InboundPendingResponse;
                        NewRequest.ProductUserId = GetUserIdString(RemoteUserId);
                        NewRequest.NextAttempt = FDateTime::MinValue();
                        FFriendSystemImpl::AddPendingFriendRequest(
                            State,
                            RemoteUserId,
                            MoveTemp(NewRequest),
                            TEXT("OnPresenceInboundMessageConnectionEstablished received message of type "
                                 "FriendRequest."));
                    }
                    State->bDirty = true;

                    this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                        .Next(IfBoundValue(
                            this,
                            [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                LocalUserId,
                                RemoteUserId,
#endif
                                AckMessage](const FError &ErrorCode) {
                                if (!ErrorCode.WasSuccessful())
                                {
                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Warning,
                                        TEXT("Unable to save cross-platform friends database in response to incoming "
                                             "friend request: %s"),
                                        *ErrorCode.ToLogString());
                                    return;
                                }

                                // Ack the message so the remote knows that we have now successfully tracked the friend
                                // request.
                                AckMessage.ExecuteIfBound();

                                UE_LOG(
                                    LogRedpointEOSFriends,
                                    Verbose,
                                    TEXT("Successfully processed incoming friend request from %s to %s."),
                                    *GetUserIdString(RemoteUserId),
                                    *GetUserIdString(LocalUserId));
                            },
                            [](const FError &) {
                            }));
                }
                else if (MessageType.IsEqual(FriendMessageFriendRequestReject))
                {
                    auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
                    if (ExistingRequest == nullptr)
                    {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("%s"),
                            *Errors::UnexpectedError(
                                 ANSI_TO_TCHAR(__FUNCTION__),
                                 FString::Printf(
                                     TEXT("Received a friend request rejection from '%s', but we are not currently "
                                          "processing a friend request to this user. The message will be dropped."),
                                     *GetUserIdString(RemoteUserId)))
                                 .ToLogString());
                        return;
                    }

                    switch (ExistingRequest->Mode)
                    {
                    case ESerializedPendingFriendRequestMode::OutboundPendingSend:
                    case ESerializedPendingFriendRequestMode::__NoLongerUsed_OutboundPendingReceive:
                    case ESerializedPendingFriendRequestMode::OutboundPendingResponse: {
                        FFriendSystemImpl::RemovePendingFriendRequest(
                            State,
                            RemoteUserId,
                            TEXT("OnPresenceInboundMessageConnectionEstablished received RejectRequest and existing "
                                 "mode is OutboundPendingSend or OutboundPendingResponse."));
                        State->bDirty = true;

                        this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                            .Next(IfBoundValue(
                                this,
                                [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                    LocalUserId,
                                    RemoteUserId,
#endif
                                    AckMessage](const FError &ErrorCode) {
                                    if (!ErrorCode.WasSuccessful())
                                    {
                                        UE_LOG(
                                            LogRedpointEOSFriends,
                                            Warning,
                                            TEXT("Unable to save cross-platform friends database in response to "
                                                 "rejected friend request: %s"),
                                            *ErrorCode.ToLogString());
                                        return;
                                    }

                                    // Ack the message so the remote knows that we have seen the rejection.
                                    AckMessage.ExecuteIfBound();

                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Verbose,
                                        TEXT("Successfully handled friend request rejection from %s to %s."),
                                        *GetUserIdString(RemoteUserId),
                                        *GetUserIdString(LocalUserId));
                                },
                                [](const FError &) {
                                }));
                        break;
                    }
                    case ESerializedPendingFriendRequestMode::InboundPendingResponse:
                    case ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance:
                    case ESerializedPendingFriendRequestMode::InboundPendingSendRejection:
                    case ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion:
                    default: {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("%s"),
                            *Errors::UnexpectedError(
                                 ANSI_TO_TCHAR(__FUNCTION__),
                                 FString::Printf(
                                     TEXT("Ignoring invalid friend request rejection message for friend that was not "
                                          "in the correct state. Sent by %s to %s."),
                                     *GetUserIdString(RemoteUserId),
                                     *GetUserIdString(LocalUserId)))
                                 .ToLogString());
                        return;
                    }
                    }
                }
                else if (MessageType.IsEqual(FriendMessageFriendRequestAccept))
                {
                    auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
                    if (ExistingRequest == nullptr)
                    {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("%s"),
                            *Errors::UnexpectedError(
                                 ANSI_TO_TCHAR(__FUNCTION__),
                                 FString::Printf(
                                     TEXT("Received a friend request acceptance from '%s', but we are not currently "
                                          "processing a friend request to this user. The message will be dropped."),
                                     *GetUserIdString(RemoteUserId)))
                                 .ToLogString());
                        return;
                    }

                    switch (ExistingRequest->Mode)
                    {
                    case ESerializedPendingFriendRequestMode::OutboundPendingSend:
                    case ESerializedPendingFriendRequestMode::__NoLongerUsed_OutboundPendingReceive:
                    case ESerializedPendingFriendRequestMode::OutboundPendingResponse: {
                        FSerializedFriend NewFriend = {};
                        NewFriend.ProductUserId = GetUserIdString(RemoteUserId);
                        State->FriendDatabase->Friends.Add(RemoteUserId, NewFriend);
                        FFriendSystemImpl::RemovePendingFriendRequest(
                            State,
                            RemoteUserId,
                            TEXT("OnPresenceInboundMessageConnectionEstablished received RequestAccept and existing "
                                 "mode is OutboundPendingSend or OutboundPendingResponse."));
                        State->bDirty = true;

                        this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                            .Next(IfBoundValue(
                                this,
                                [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                    LocalUserId,
                                    RemoteUserId,
#endif
                                    AckMessage](const FError &ErrorCode) {
                                    if (!ErrorCode.WasSuccessful())
                                    {
                                        UE_LOG(
                                            LogRedpointEOSFriends,
                                            Warning,
                                            TEXT("Unable to save cross-platform friends database in response to "
                                                 "accepted friend request: %s"),
                                            *ErrorCode.ToLogString());
                                        return;
                                    }

                                    // Ack the message so the remote knows that we have seen the acceptance.
                                    AckMessage.ExecuteIfBound();

                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Verbose,
                                        TEXT("Successfully handled friend request acceptance from %s to %s."),
                                        *GetUserIdString(RemoteUserId),
                                        *GetUserIdString(LocalUserId));
                                },
                                [](const FError &) {
                                }));
                        break;
                    }
                    case ESerializedPendingFriendRequestMode::InboundPendingResponse:
                    case ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance:
                    case ESerializedPendingFriendRequestMode::InboundPendingSendRejection:
                    case ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion:
                    default: {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("%s"),
                            *Errors::UnexpectedError(
                                 ANSI_TO_TCHAR(__FUNCTION__),
                                 FString::Printf(
                                     TEXT("Ignoring invalid friend request rejection message for friend that was not "
                                          "in the correct state. Sent by %s to %s."),
                                     *GetUserIdString(RemoteUserId),
                                     *GetUserIdString(LocalUserId)))
                                 .ToLogString());
                        return;
                    }
                    }
                }
                else if (MessageType.IsEqual(FriendMessageFriendDelete))
                {
                    bool bDidModify = false;

                    auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
                    if (ExistingRequest != nullptr)
                    {
                        FFriendSystemImpl::RemovePendingFriendRequest(
                            State,
                            RemoteUserId,
                            TEXT("OnPresenceInboundMessageConnectionEstablished received FriendDelete and existing "
                                 "mode is anything."));
                        bDidModify = true;
                    }

                    auto *ExistingFriend = State->FriendDatabase->Friends.Find(RemoteUserId);
                    if (ExistingFriend != nullptr)
                    {
                        State->FriendDatabase->Friends.Remove(RemoteUserId);
                        bDidModify = true;
                    }

                    if (!bDidModify)
                    {
                        // We didn't need to actually change anything. Just ack.
                        AckMessage.ExecuteIfBound();
                        return;
                    }

                    State->bDirty = true;

                    this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                        .Next(IfBoundValue(
                            this,
                            [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                LocalUserId,
                                RemoteUserId,
#endif
                                AckMessage](const FError &ErrorCode) {
                                if (!ErrorCode.WasSuccessful())
                                {
                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Warning,
                                        TEXT("Unable to save cross-platform friends database in response to "
                                             "deleted friend: %s"),
                                        *ErrorCode.ToLogString());
                                    return;
                                }

                                // Ack the message so the remote knows that we have seen the deletion request.
                                AckMessage.ExecuteIfBound();

                                UE_LOG(
                                    LogRedpointEOSFriends,
                                    Verbose,
                                    TEXT("Successfully handled friend deletion from %s to %s."),
                                    *GetUserIdString(RemoteUserId),
                                    *GetUserIdString(LocalUserId));
                            },
                            [](const FError &) {
                            }));
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSFriends,
                        Error,
                        TEXT("Unable to process MESSAGE for local user '%s' from remote user '%s', the message type "
                             "'%s' is unknown!"),
                        *GetUserIdString(LocalUserId),
                        *GetUserIdString(RemoteUserId),
                        *MessageType.ToString());
                }
            }));
}

void FFriendSystemImpl::OnRemoteUserPresenceStateUpdated(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &TargetUserId,
    const FPresenceUserState &NewPresenceState)
{
    UE_LOG(
        LogRedpointEOSFriends,
        Verbose,
        TEXT("Received notification for %s that remote user %s presence has changed."),
        *GetUserIdString(LocalUserId),
        *GetUserIdString(TargetUserId));

    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        UE_LOG(
            LogRedpointEOSFriends,
            Warning,
            TEXT("Can not propagate notification for %s that remote user %s presence has changed, because the local "
                 "user doesn't have a friend system state."),
            *GetUserIdString(LocalUserId),
            *GetUserIdString(TargetUserId));
        return;
    }
    auto &State = *StatePtr;

    if (State->UnifiedFriends.Contains(TargetUserId) || State->InboundFriendRequests.Contains(TargetUserId) ||
        State->OutboundFriendRequests.Contains(TargetUserId))
    {
        UE_LOG(
            LogRedpointEOSFriends,
            Verbose,
            TEXT("Announcing notification for %s that remote user %s presence has changed."),
            *GetUserIdString(LocalUserId),
            *GetUserIdString(TargetUserId));
        State->UnifiedFriendsPresenceUserState.Add(TargetUserId, NewPresenceState);
        this->OnFriendPresenceChanged().Broadcast(LocalUserId, TargetUserId, NewPresenceState);
    }
    else
    {
        UE_LOG(
            LogRedpointEOSFriends,
            Warning,
            TEXT("Can not propagate notification for %s that remote user %s presence has changed, because the target "
                 "user isn't tracked as a friend or friend request."),
            *GetUserIdString(LocalUserId),
            *GetUserIdString(TargetUserId));
    }
}

void FFriendSystemImpl::OnPeriodicTimerQueuePendingOperations()
{
    for (const auto &KV : this->LocalUserFriendStates)
    {
        // Flush cross-platform friends database if needed.
        if (KV.Value->bDirty)
        {
            this->Queue(KV.Key, EOperationType::Save | EOperationType::Refresh).Next([](const FError &ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogRedpointEOSFriends,
                        Warning,
                        TEXT("Periodic database save and refresh failed with error: %s"),
                        *ErrorCode.ToLogString());
                }
            });
        }

        // Attempt to schedule any pending friend request operations as needed (these won't actually happen if
        // NextAttempt is still in the future).
        for (const auto &PendingRequest : KV.Value->FriendDatabase->PendingFriendRequests)
        {
            switch (PendingRequest.Value.Mode)
            {
            case ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance:
                this->QueueSendFriendAcceptanceAndWait(KV.Key, PendingRequest.Key).Next([](const FError &ErrorCode) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("Retry of send friend acceptance operation failed with error: %s"),
                            *ErrorCode.ToLogString());
                    }
                });
                break;
            case ESerializedPendingFriendRequestMode::InboundPendingSendRejection:
                this->QueueSendFriendRejectionAndWait(KV.Key, PendingRequest.Key).Next([](const FError &ErrorCode) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("Retry of send friend rejection operation failed with error: %s"),
                            *ErrorCode.ToLogString());
                    }
                });
                break;
            case ESerializedPendingFriendRequestMode::OutboundPendingSend:
                this->QueueSendFriendRequestAndWait(KV.Key, PendingRequest.Key).Next([](const FError &ErrorCode) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("Retry of send friend request operation failed with error: %s"),
                            *ErrorCode.ToLogString());
                    }
                });
                break;
            case ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion:
                this->QueueSendFriendDeletionAndWait(KV.Key, PendingRequest.Key).Next([](const FError &ErrorCode) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("Retry of send friend deletion operation failed with error: %s"),
                            *ErrorCode.ToLogString());
                    }
                });
                break;
            case ESerializedPendingFriendRequestMode::__NoLongerUsed_OutboundPendingReceive:
                // This mode is no longer used.
                break;
            case ESerializedPendingFriendRequestMode::OutboundPendingResponse:
            case ESerializedPendingFriendRequestMode::InboundPendingResponse:
                // We're waiting on the user to take an action.
                break;
            default:
                // Unhandled.
                checkf(false, TEXT("Pending friend request in unknown mode!"));
                break;
            }
        }
    }
}

void FFriendSystemImpl::DeferPendingFriendRequestOperation(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return;
    }
    auto &State = *StatePtr;

    auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
    if (ExistingRequest != nullptr)
    {
        ExistingRequest->NextAttempt = FDateTime::UtcNow() + FTimespan::FromMinutes(10);
        State->bDirty = true;
    }
    return;
}

TFuture<FError> FFriendSystemImpl::QueueSendFriendRequestAndWait(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    return this->OperationQueue->AddToQueue(this, [this, LocalUserId, RemoteUserId]() -> TFuture<FError> {
        auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
        if (StatePtr == nullptr)
        {
            return MakeFulfilledPromise<FError>(
                       Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")))
                .GetFuture();
        }
        auto &State = *StatePtr;

        auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
        if (ExistingRequest == nullptr)
        {
            return MakeFulfilledPromise<FError>(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("The queued friend request no longer exists.")))
                .GetFuture();
        }

        if (ExistingRequest->Mode != ESerializedPendingFriendRequestMode::OutboundPendingSend)
        {
            return MakeFulfilledPromise<FError>(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("The queued friend request is no longer pending send.")))
                .GetFuture();
        }

        if (FDateTime::UtcNow() < ExistingRequest->NextAttempt)
        {
            // We're not ready to send this yet.
            return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
        }

        // Attempt to open a connection to the remote user and send the friend request.
        UE_LOG(
            LogRedpointEOSFriends,
            VeryVerbose,
            TEXT("Attempting to open a remote presence connection from local user '%s' to remote user '%s'..."),
            *GetUserIdString(LocalUserId),
            *GetUserIdString(RemoteUserId));
        auto Promise = MakeShared<TPromise<FError>>();
        auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
        PresenceSystem->TryOpenMessageConnection(
            LocalUserId,
            RemoteUserId,
            FriendSystemName,
            IPresenceSystem::FOnTryOpenMessageConnection::CreateSPLambda(
                this,
                [this, LocalUserId, RemoteUserId, Promise, State](
                    const FError &ErrorCode,
                    const TSharedPtr<IPresenceMessageConnection> &MessageConnection) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        // Can't send right now. Keep it in the queue.
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Error,
                            TEXT("Failed to open a remote presence connection from local user '%s' to remote user "
                                 "'%s'."),
                            *GetUserIdString(LocalUserId),
                            *GetUserIdString(RemoteUserId));
                        this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                        Promise->SetValue(Errors::Success());
                        return;
                    }

                    UE_LOG(
                        LogRedpointEOSFriends,
                        VeryVerbose,
                        TEXT("Successfully opened a remote presence connection from local user '%s' to remote user "
                             "'%s', attempting to send desired message..."),
                        *GetUserIdString(LocalUserId),
                        *GetUserIdString(RemoteUserId));
                    checkf(MessageConnection.IsValid(), TEXT("Connection should be valid!"));
                    MessageConnection->SendMessage(
                        FriendMessageFriendRequest,
                        MakeShared<FJsonValueBoolean>(true),
                        IPresenceMessageConnection::FOnSendMessageComplete::CreateSPLambda(
                            this,
                            [this, MessageConnection, LocalUserId, RemoteUserId, State, Promise](
                                const FError &SendError) {
                                if (!SendError.WasSuccessful())
                                {
                                    // They didn't ack it right now. Keep it in the queue.
                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Error,
                                        TEXT("The remote user '%s' did not ack our message within the expected "
                                             "timeout."),
                                        *GetUserIdString(RemoteUserId));
                                    this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                                    MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete());
                                    Promise->SetValue(Errors::Success());
                                    return;
                                }

                                UE_LOG(
                                    LogRedpointEOSFriends,
                                    VeryVerbose,
                                    TEXT("Successfully received ack for outbound message over remote presence "
                                         "connection from local user '%s' to remote user '%s'."),
                                    *GetUserIdString(LocalUserId),
                                    *GetUserIdString(RemoteUserId));

                                // The remote user received our request and saved it to their cross-platform friends
                                // database.
                                auto *FoundFriendRequest =
                                    State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
                                if (FoundFriendRequest != nullptr &&
                                    FoundFriendRequest->Mode ==
                                        ESerializedPendingFriendRequestMode::OutboundPendingSend)
                                {
                                    FoundFriendRequest->Mode =
                                        ESerializedPendingFriendRequestMode::OutboundPendingResponse;
                                }
                                State->bDirty = true;

                                this->Immediate(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                                    .Next(IfBoundValue(
                                        this,
                                        [this, MessageConnection, LocalUserId, RemoteUserId, State, Promise](
                                            const FError &SaveErrorCode) {
                                            // Close the connection.
                                            UE_LOG(
                                                LogRedpointEOSFriends,
                                                VeryVerbose,
                                                TEXT("Closing presence connection from local user '%s' to remote user "
                                                     "'%s'."),
                                                *GetUserIdString(LocalUserId),
                                                *GetUserIdString(RemoteUserId));
                                            MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete());

                                            // If we fail to save, we'll try to send the friend request again
                                            // and get an ack for it.
                                            if (!SaveErrorCode.WasSuccessful())
                                            {
                                                this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                                                UE_LOG(
                                                    LogRedpointEOSFriends,
                                                    Error,
                                                    TEXT("Unable to save cross-platform friends database to after "
                                                         "sending friend request!"));
                                                Promise->SetValue(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("Unable to save cross-platform friends database to after "
                                                         "sending friend request!")));
                                                return;
                                            }

                                            // We're done.
                                            Promise->SetValue(Errors::Success());
                                        },
                                        [Promise](const FError &) {
                                            Promise->SetValue(Errors::Canceled(
                                                ANSI_TO_TCHAR(__FUNCTION__),
                                                TEXT("The friends system was shutdown before this call could "
                                                     "complete.")));
                                        }));
                            }));
                }));
        return Promise->GetFuture();
    });
}

TFuture<FError> FFriendSystemImpl::QueueSendFriendAcceptanceAndWait(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    UE_LOG(LogRedpointEOSFriends, VeryVerbose, TEXT("Queueing send of friend acceptance message."));
    return this->OperationQueue->AddToQueue(this, [this, LocalUserId, RemoteUserId]() -> TFuture<FError> {
        auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
        if (StatePtr == nullptr)
        {
            return MakeFulfilledPromise<FError>(
                       Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")))
                .GetFuture();
        }
        auto &State = *StatePtr;

        auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
        if (ExistingRequest == nullptr)
        {
            return MakeFulfilledPromise<FError>(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("The queued friend request no longer exists.")))
                .GetFuture();
        }

        if (ExistingRequest->Mode != ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance)
        {
            return MakeFulfilledPromise<FError>(
                       Errors::UnexpectedError(
                           ANSI_TO_TCHAR(__FUNCTION__),
                           TEXT("The queued friend request is no longer pending send of acceptance.")))
                .GetFuture();
        }

        if (FDateTime::UtcNow() < ExistingRequest->NextAttempt)
        {
            // We're not ready to send this yet.
            return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
        }

        // Attempt to open a connection to the remote user and send the friend acceptance.
        auto Promise = MakeShared<TPromise<FError>>();
        auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
        UE_LOG(
            LogRedpointEOSFriends,
            VeryVerbose,
            TEXT("Attempting to open a presence remote connection to accept friend request..."));
        PresenceSystem->TryOpenMessageConnection(
            LocalUserId,
            RemoteUserId,
            FriendSystemName,
            IPresenceSystem::FOnTryOpenMessageConnection::CreateSPLambda(
                this,
                [this, LocalUserId, RemoteUserId, Promise, State](
                    const FError &ErrorCode,
                    const TSharedPtr<IPresenceMessageConnection> &MessageConnection) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        // Can't send right now. Keep it in the queue.
                        this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                        Promise->SetValue(Errors::Success());
                        return;
                    }

                    checkf(MessageConnection.IsValid(), TEXT("Connection should be valid!"));
                    MessageConnection->SendMessage(
                        FriendMessageFriendRequestAccept,
                        MakeShared<FJsonValueBoolean>(true),
                        IPresenceMessageConnection::FOnSendMessageComplete::CreateSPLambda(
                            this,
                            [this, MessageConnection, LocalUserId, RemoteUserId, State, Promise](
                                const FError &SendError) {
                                if (!SendError.WasSuccessful())
                                {
                                    // They didn't ack it right now. Keep it in the queue.
                                    this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                                    MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete());
                                    Promise->SetValue(Errors::Success());
                                    return;
                                }

                                // The remote user received our request and processed it.
                                FFriendSystemImpl::RemovePendingFriendRequest(
                                    State,
                                    RemoteUserId,
                                    TEXT("Target user successfully processed our RequestAccept message."));
                                State->bDirty = true;

                                this->Immediate(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                                    .Next(IfBoundValue(
                                        this,
                                        [this, MessageConnection, LocalUserId, RemoteUserId, State, Promise](
                                            const FError &SaveErrorCode) {
                                            // Close the connection.
                                            MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete());

                                            // If we fail to save, we'll try to send the friend acceptance again
                                            // and get an ack for it.
                                            if (!SaveErrorCode.WasSuccessful())
                                            {
                                                this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                                                UE_LOG(
                                                    LogRedpointEOSFriends,
                                                    Error,
                                                    TEXT("Unable to save cross-platform friends database to after "
                                                         "sending friend acceptance!"));
                                                Promise->SetValue(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("Unable to save cross-platform friends database to after "
                                                         "sending friend acceptance!")));
                                                return;
                                            }

                                            // We're done.
                                            Promise->SetValue(Errors::Success());
                                        },
                                        [Promise](const FError &) {
                                            Promise->SetValue(Errors::Canceled(
                                                ANSI_TO_TCHAR(__FUNCTION__),
                                                TEXT("The friends system was shutdown before this call could "
                                                     "complete.")));
                                        }));
                            }));
                }));
        return Promise->GetFuture();
    });
}

TFuture<FError> FFriendSystemImpl::QueueSendFriendRejectionAndWait(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    return this->OperationQueue->AddToQueue(this, [this, LocalUserId, RemoteUserId]() -> TFuture<FError> {
        auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
        if (StatePtr == nullptr)
        {
            return MakeFulfilledPromise<FError>(
                       Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")))
                .GetFuture();
        }
        auto &State = *StatePtr;

        auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
        if (ExistingRequest == nullptr)
        {
            return MakeFulfilledPromise<FError>(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("The queued friend request no longer exists.")))
                .GetFuture();
        }

        if (ExistingRequest->Mode != ESerializedPendingFriendRequestMode::InboundPendingSendRejection)
        {
            return MakeFulfilledPromise<FError>(
                       Errors::UnexpectedError(
                           ANSI_TO_TCHAR(__FUNCTION__),
                           TEXT("The queued friend request is no longer pending send of acceptance.")))
                .GetFuture();
        }

        if (FDateTime::UtcNow() < ExistingRequest->NextAttempt)
        {
            // We're not ready to send this yet.
            return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
        }

        // Attempt to open a connection to the remote user and send the friend rejection.
        auto Promise = MakeShared<TPromise<FError>>();
        auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
        PresenceSystem->TryOpenMessageConnection(
            LocalUserId,
            RemoteUserId,
            FriendSystemName,
            IPresenceSystem::FOnTryOpenMessageConnection::CreateSPLambda(
                this,
                [this, LocalUserId, RemoteUserId, Promise, State](
                    const FError &ErrorCode,
                    const TSharedPtr<IPresenceMessageConnection> &MessageConnection) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        // Can't send right now. Keep it in the queue.
                        this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                        Promise->SetValue(Errors::Success());
                        return;
                    }

                    checkf(MessageConnection.IsValid(), TEXT("Connection should be valid!"));
                    MessageConnection->SendMessage(
                        FriendMessageFriendRequestReject,
                        MakeShared<FJsonValueBoolean>(true),
                        IPresenceMessageConnection::FOnSendMessageComplete::CreateSPLambda(
                            this,
                            [this, MessageConnection, LocalUserId, RemoteUserId, State, Promise](
                                const FError &SendError) {
                                if (!SendError.WasSuccessful())
                                {
                                    // They didn't ack it right now. Keep it in the queue.
                                    this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                                    MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete());
                                    Promise->SetValue(Errors::Success());
                                    return;
                                }

                                // The remote user received our request and processed it.
                                FFriendSystemImpl::RemovePendingFriendRequest(
                                    State,
                                    RemoteUserId,
                                    TEXT("Target user successfully processed our RequestReject message."));
                                State->bDirty = true;

                                this->Immediate(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                                    .Next(IfBoundValue(
                                        this,
                                        [this, MessageConnection, LocalUserId, RemoteUserId, State, Promise](
                                            const FError &SaveErrorCode) {
                                            // Close the connection.
                                            MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete());

                                            // If we fail to save, we'll try to send the friend rejection again
                                            // and get an ack for it.
                                            if (!SaveErrorCode.WasSuccessful())
                                            {
                                                this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                                                UE_LOG(
                                                    LogRedpointEOSFriends,
                                                    Error,
                                                    TEXT("Unable to save cross-platform friends database to after "
                                                         "sending friend rejection!"));
                                                Promise->SetValue(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("Unable to save cross-platform friends database to after "
                                                         "sending friend rejection!")));
                                                return;
                                            }

                                            // We're done.
                                            Promise->SetValue(Errors::Success());
                                        },
                                        [Promise](const FError &) {
                                            Promise->SetValue(Errors::Canceled(
                                                ANSI_TO_TCHAR(__FUNCTION__),
                                                TEXT("The friends system was shutdown before this call could "
                                                     "complete.")));
                                        }));
                            }));
                }));
        return Promise->GetFuture();
    });
}

TFuture<FError> FFriendSystemImpl::QueueSendFriendDeletionAndWait(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    return this->OperationQueue->AddToQueue(this, [this, LocalUserId, RemoteUserId]() -> TFuture<FError> {
        auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
        if (StatePtr == nullptr)
        {
            return MakeFulfilledPromise<FError>(
                       Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")))
                .GetFuture();
        }
        auto &State = *StatePtr;

        auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
        if (ExistingRequest == nullptr)
        {
            return MakeFulfilledPromise<FError>(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("The queued friend request no longer exists.")))
                .GetFuture();
        }

        if (ExistingRequest->Mode != ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion)
        {
            return MakeFulfilledPromise<FError>(
                       Errors::UnexpectedError(
                           ANSI_TO_TCHAR(__FUNCTION__),
                           TEXT("The queued friend request is no longer pending send of deletion.")))
                .GetFuture();
        }

        if (FDateTime::UtcNow() < ExistingRequest->NextAttempt)
        {
            // We're not ready to send this yet.
            return MakeFulfilledPromise<FError>(Errors::Success()).GetFuture();
        }

        // Attempt to open a connection to the remote user and send the friend deletion.
        auto Promise = MakeShared<TPromise<FError>>();
        auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
        PresenceSystem->TryOpenMessageConnection(
            LocalUserId,
            RemoteUserId,
            FriendSystemName,
            IPresenceSystem::FOnTryOpenMessageConnection::CreateSPLambda(
                this,
                [this, LocalUserId, RemoteUserId, Promise, State](
                    const FError &ErrorCode,
                    const TSharedPtr<IPresenceMessageConnection> &MessageConnection) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        // Can't send right now. Keep it in the queue.
                        this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                        Promise->SetValue(Errors::Success());
                        return;
                    }

                    checkf(MessageConnection.IsValid(), TEXT("Connection should be valid!"));
                    MessageConnection->SendMessage(
                        FriendMessageFriendDelete,
                        MakeShared<FJsonValueBoolean>(true),
                        IPresenceMessageConnection::FOnSendMessageComplete::CreateSPLambda(
                            this,
                            [this, MessageConnection, LocalUserId, RemoteUserId, State, Promise](
                                const FError &SendError) {
                                if (!SendError.WasSuccessful())
                                {
                                    // They didn't ack it right now. Keep it in the queue.
                                    this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                                    MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete());
                                    Promise->SetValue(Errors::Success());
                                    return;
                                }

                                // The remote user received our request and processed it.
                                FFriendSystemImpl::RemovePendingFriendRequest(
                                    State,
                                    RemoteUserId,
                                    TEXT("Target user successfully processed our FriendDelete message."));
                                State->bDirty = true;

                                this->Immediate(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                                    .Next(IfBoundValue(
                                        this,
                                        [this, MessageConnection, LocalUserId, RemoteUserId, State, Promise](
                                            const FError &SaveErrorCode) {
                                            // Close the connection.
                                            MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete());

                                            // If we fail to save, we'll try to send the friend deletion again
                                            // and get an ack for it.
                                            if (!SaveErrorCode.WasSuccessful())
                                            {
                                                this->DeferPendingFriendRequestOperation(LocalUserId, RemoteUserId);
                                                UE_LOG(
                                                    LogRedpointEOSFriends,
                                                    Error,
                                                    TEXT("Unable to save cross-platform friends database to after "
                                                         "sending friend deletion!"));
                                                Promise->SetValue(Errors::UnexpectedError(
                                                    ANSI_TO_TCHAR(__FUNCTION__),
                                                    TEXT("Unable to save cross-platform friends database to after "
                                                         "sending friend deletion!")));
                                                return;
                                            }

                                            // We're done.
                                            Promise->SetValue(Errors::Success());
                                        },
                                        [Promise](const FError &) {
                                            Promise->SetValue(Errors::Canceled(
                                                ANSI_TO_TCHAR(__FUNCTION__),
                                                TEXT("The friends system was shutdown before this call could "
                                                     "complete.")));
                                        }));
                            }));
                }));
        return Promise->GetFuture();
    });
}

void FFriendSystemImpl::UpdateNonUnifiedFriendPresence(
    int LocalUserNum,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId,
    EPresenceUserActivityState NewPresenceActivityState)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto LocalUser = IdentitySystem->GetUser(LocalUserNum);
    if (!LocalUser.IsValid())
    {
        return;
    }

    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUser->GetUserId());
    if (StatePtr == nullptr)
    {
        return;
    }
    auto &State = *StatePtr;

    for (const auto &NonUnifiedFriend : State->NonUnifiedFriends)
    {
        auto *RelatedId = NonUnifiedFriend->GetLinkedExternalAccounts().Find(FriendAccountType);
        if (RelatedId != nullptr && RelatedId->AccountId == FriendAccountId)
        {
            State->NonUnifiedFriendsPresenceUserActivityState.Add(
                TTuple<EOS_EExternalAccountType, FString>(FriendAccountType, FriendAccountId),
                NewPresenceActivityState);
            this->OnNonUnifiedFriendPresenceChanged().Broadcast(
                LocalUser->GetUserId(),
                NonUnifiedFriend,
                NewPresenceActivityState);

            // @note: Only update one friend for performance - we shouldn't have multiple friends with the same linked
            // account ID anyway.
            return;
        }
    }
}

IFriendSystem::FOnFriendsInitiallyLoaded &FFriendSystemImpl::OnFriendsInitiallyLoaded()
{
    return this->FriendsInitiallyLoadedDelegate;
}

IFriendSystem::FOnFriendsChanged &FFriendSystemImpl::OnFriendsChanged()
{
    return this->FriendsChangedDelegate;
}

IFriendSystem::FOnFriendPresenceChanged &FFriendSystemImpl::OnFriendPresenceChanged()
{
    return this->FriendPresenceChangedDelegate;
}

IFriendSystem::FOnNonUnifiedFriendPresenceChanged &FFriendSystemImpl::OnNonUnifiedFriendPresenceChanged()
{
    return this->NonUnifiedFriendPresenceChangedDelegate;
}

IFriendSystem::FOnFriendRequestReceived &FFriendSystemImpl::OnFriendRequestReceived()
{
    return this->FriendRequestReceivedDelegate;
}

FError FFriendSystemImpl::GetFriends(
    const FIdentityUserId &LocalUserId,
    TAccountIdMap<FUserInfoRef> &OutUnifiedFriends,
    TArray<FExternalUserInfoRef> &OutNonUnifiedFriends)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in."));
    }
    auto &State = *StatePtr;

    OutUnifiedFriends = State->UnifiedFriends;
    OutNonUnifiedFriends = State->NonUnifiedFriends;
    return Errors::Success();
}

TOptional<FPresenceUserState> FFriendSystemImpl::GetUnifiedFriendPresence(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &FriendUserId) const
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return TOptional<FPresenceUserState>();
    }
    auto &State = *StatePtr;

    auto *UnifiedFriendPresence = State->UnifiedFriendsPresenceUserState.Find(FriendUserId);
    if (UnifiedFriendPresence == nullptr)
    {
        return TOptional<FPresenceUserState>();
    }
    return *UnifiedFriendPresence;
}

TOptional<EPresenceUserActivityState> FFriendSystemImpl::GetNonUnifiedFriendPresence(
    const FIdentityUserId &LocalUserId,
    const FExternalUserInfoRef &FriendUser) const
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return TOptional<EPresenceUserActivityState>();
    }
    auto &State = *StatePtr;

    for (const auto &LinkedAccount : FriendUser->GetLinkedExternalAccounts())
    {
        auto *PresenceActivityState = State->NonUnifiedFriendsPresenceUserActivityState.Find(
            TTuple<EOS_EExternalAccountType, FString>(LinkedAccount.Key, LinkedAccount.Value.AccountId));
        if (PresenceActivityState != nullptr)
        {
            return *PresenceActivityState;
        }
    }

    return TOptional<EPresenceUserActivityState>();
}

FError FFriendSystemImpl::GetInboundFriendRequests(
    const FIdentityUserId &LocalUserId,
    TAccountIdMap<FUserInfoRef> &OutInboundFriendRequests)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in."));
    }
    auto &State = *StatePtr;

    OutInboundFriendRequests = State->InboundFriendRequests;
    return Errors::Success();
}

FError FFriendSystemImpl::GetOutboundFriendRequests(
    const FIdentityUserId &LocalUserId,
    TAccountIdMap<FUserInfoRef> &OutOutboundFriendRequests)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in."));
    }
    auto &State = *StatePtr;

    OutOutboundFriendRequests = State->OutboundFriendRequests;
    return Errors::Success();
}

TFuture<IFriendSystem::FSendFriendRequestResult> FFriendSystemImpl::SendFriendRequest(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return FSendFriendRequestResult::Future(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")),
            LocalUserId,
            RemoteUserId);
    }
    auto &State = *StatePtr;

    auto *ExistingFriend = State->FriendDatabase->Friends.Find(RemoteUserId);
    if (ExistingFriend != nullptr)
    {
        // The user is already a friend.
        // @note: Should this be a success case?
        return FSendFriendRequestResult::Future(Errors::Success(), LocalUserId, RemoteUserId);
    }

    auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteUserId);
    if (ExistingRequest != nullptr)
    {
        switch (ExistingRequest->Mode)
        {
        case ESerializedPendingFriendRequestMode::OutboundPendingSend:
        case ESerializedPendingFriendRequestMode::__NoLongerUsed_OutboundPendingReceive:
        case ESerializedPendingFriendRequestMode::OutboundPendingResponse: {
            // We're already sending this person a friend request.
            return FSendFriendRequestResult::Future(Errors::Success(), LocalUserId, RemoteUserId);
        }
        case ESerializedPendingFriendRequestMode::InboundPendingResponse:
        case ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance:
        case ESerializedPendingFriendRequestMode::InboundPendingSendRejection: {
            // If you send a friend request to someone who has sent you a friend request (and they haven't seen the
            // outcome yet), this is the same thing as calling AcceptFriendRequest.
            return this->AcceptFriendRequest(LocalUserId, RemoteUserId).Next([](const auto &AcceptResult) {
                return FSendFriendRequestResult(
                    AcceptResult.ErrorCode,
                    AcceptResult.LocalUserId,
                    AcceptResult.RemoteRequesterUserId);
            });
        }
        case ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion: {
            // Undo deletion.
            FSerializedFriend RestoredFriend = {};
            RestoredFriend.ProductUserId = GetUserIdString(RemoteUserId);
            State->FriendDatabase->Friends.Add(RemoteUserId, RestoredFriend);
            FFriendSystemImpl::RemovePendingFriendRequest(
                State,
                RemoteUserId,
                TEXT("SendFriendRequest with an existing request in OutboundPendingSendDeletion."));
            State->bDirty = true;
            return this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
                .Next([RemoteUserId, LocalUserId](FError ErrorCode) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Warning,
                            TEXT("Unable to save cross-platform friends database in response to "
                                 "restoring a pending friend deletion: %s"),
                            *ErrorCode.ToLogString());
                    }
                    else
                    {
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Verbose,
                            TEXT("Successfully cancelled friend deletion from %s to %s."),
                            *GetUserIdString(RemoteUserId),
                            *GetUserIdString(LocalUserId));
                    }
                    return FSendFriendRequestResult(ErrorCode, LocalUserId, RemoteUserId);
                });
        }
        default: {
            checkf(false, TEXT("Friend request in an invalid state for SendFriendRequest."));
            return FSendFriendRequestResult::Future(
                Errors::UnexpectedError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("The existing friend request was in an invalid state that SendFriendRequest can't handle.")),
                LocalUserId,
                RemoteUserId);
        }
        }
    }

    // Track the friend request.
    {
        FSerializedPendingFriendRequest FriendRequest = {};
        FriendRequest.ProductUserId = GetUserIdString(RemoteUserId);
        FriendRequest.Mode = ESerializedPendingFriendRequestMode::OutboundPendingSend;
        FriendRequest.NextAttempt = FDateTime::MinValue();
        FFriendSystemImpl::AddPendingFriendRequest(
            State,
            RemoteUserId,
            MoveTemp(FriendRequest),
            TEXT("SendFriendRequest was called."));
    }
    State->bDirty = true;

    // Save the pending friend request before we attempt to send.
    return (this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh) +
            IfBoundAwait(
                this,
                [this, LocalUserId, RemoteUserId, State](const FError &SaveErrorCode) -> TFuture<FError> {
                    if (!SaveErrorCode.WasSuccessful())
                    {
                        // We couldn't even save it.
                        FFriendSystemImpl::RemovePendingFriendRequest(
                            State,
                            RemoteUserId,
                            TEXT("Failed to save database during SendFriendRequest."));
                        State->bDirty = true;
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Error,
                            TEXT("Unable to save cross-platform friends database to queue friend request "
                                 "for later sending."));
                        return MakeFulfilledPromise<FError>(
                                   Errors::UnexpectedError(
                                       ANSI_TO_TCHAR(__FUNCTION__),
                                       TEXT("Unable to save cross-platform friends database to queue friend "
                                            "request for later sending.")))
                            .GetFuture();
                    }

                    // We've saved it so we can now safely send it.
                    return this->QueueSendFriendRequestAndWait(LocalUserId, RemoteUserId).Next([](const FError &) {
                        // It doesn't matter if the send failed, it'll continue to be in the queue even if it did.
                        return Errors::Success();
                    });
                },
                [](const FError &) -> TFuture<FError> {
                    return MakeFulfilledPromise<FError>(
                               Errors::Canceled(
                                   ANSI_TO_TCHAR(__FUNCTION__),
                                   TEXT("The friend system was shutdown before this call could complete.")))
                        .GetFuture();
                }))
        .Next([LocalUserId, RemoteUserId](FError OperationErrorCode) {
            return FSendFriendRequestResult(OperationErrorCode, LocalUserId, RemoteUserId);
        });
}

TFuture<IFriendSystem::FAcceptFriendRequestResult> FFriendSystemImpl::AcceptFriendRequest(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteRequesterUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return FAcceptFriendRequestResult::Future(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    auto &State = *StatePtr;

    auto *ExistingFriend = State->FriendDatabase->Friends.Find(RemoteRequesterUserId);
    if (ExistingFriend != nullptr)
    {
        // The user is already a friend.
        return FAcceptFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The friend request was not found (the target user is already a friend).")),
            LocalUserId,
            RemoteRequesterUserId);
    }

    auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteRequesterUserId);
    if (ExistingRequest == nullptr)
    {
        // There is no friend request from the target user.
        return FAcceptFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The friend request was not found (the target user has not sent you a friend request).")),
            LocalUserId,
            RemoteRequesterUserId);
    }

    switch (ExistingRequest->Mode)
    {
    case ESerializedPendingFriendRequestMode::OutboundPendingSend:
    case ESerializedPendingFriendRequestMode::__NoLongerUsed_OutboundPendingReceive:
    case ESerializedPendingFriendRequestMode::OutboundPendingResponse: {
        // We've sent this person a friend request - we don't have a friend request from them.
        return FAcceptFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The inbound friend request was not found (you have an outbound friend request to this user "
                     "already).")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    case ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion: {
        // We can't undo deletion with an acceptance, since we don't have a friend request to accept.
        return FAcceptFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The inbound friend request was not found (you have recently deleted this friend).")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    case ESerializedPendingFriendRequestMode::InboundPendingResponse: {
        // Accept the friend request and then queue the accept message immediately.
        ExistingRequest->Mode = ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance;
        FSerializedFriend AcceptedFriend = {};
        AcceptedFriend.ProductUserId = GetUserIdString(RemoteRequesterUserId);
        State->FriendDatabase->Friends.Add(RemoteRequesterUserId, AcceptedFriend);
        State->bDirty = true;

        // Save the pending acceptance before we attempt to send.
        UE_LOG(
            LogRedpointEOSFriends,
            VeryVerbose,
            TEXT("Queueing save and refresh before sending friend acceptance message."));
        return (this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh) +
                [this, LocalUserId, RemoteRequesterUserId, State](FError SaveErrorCode) {
                    if (!SaveErrorCode.WasSuccessful())
                    {
                        // We couldn't save it.
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Error,
                            TEXT("Unable to save cross-platform friends database to accept friend request."));
                        return MakeFulfilledPromise<FError>(Errors::UnexpectedError(
                                                                ANSI_TO_TCHAR(__FUNCTION__),
                                                                TEXT("Unable to save cross-platform friends "
                                                                     "database to accept friend request.")))
                            .GetFuture();
                    }

                    // We've saved it so we can now safely send the acceptance notification.
                    return this->QueueSendFriendAcceptanceAndWait(LocalUserId, RemoteRequesterUserId)
                        .Next([](const FError &) {
                            // It doesn't matter if sending the accept failed, it'll continue to be in the queue
                            // even if it did.
                            return Errors::Success();
                        });
                })
            .Next([LocalUserId, RemoteRequesterUserId](FError OperationErrorCode) {
                return FAcceptFriendRequestResult(OperationErrorCode, LocalUserId, RemoteRequesterUserId);
            });
    }
    case ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance: {
        // This friend request has already been accepted.
        return FAcceptFriendRequestResult::Future(Errors::Success(), LocalUserId, RemoteRequesterUserId);
    }
    case ESerializedPendingFriendRequestMode::InboundPendingSendRejection: {
        // This friend request has already been rejected.
        return FAcceptFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The inbound friend request was not found (it has already been rejected).")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    default: {
        checkf(false, TEXT("Friend request in an invalid state for SendFriendRequest."));
        return FAcceptFriendRequestResult::Future(
            Errors::UnexpectedError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The existing friend request was in an invalid state that AcceptFriendRequest can't handle.")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    }
}

TFuture<IFriendSystem::FRejectFriendRequestResult> FFriendSystemImpl::RejectFriendRequest(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteRequesterUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return FRejectFriendRequestResult::Future(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    auto &State = *StatePtr;

    auto *ExistingFriend = State->FriendDatabase->Friends.Find(RemoteRequesterUserId);
    if (ExistingFriend != nullptr)
    {
        // The user is already a friend.
        return FRejectFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The friend request was not found (the target user is already a friend).")),
            LocalUserId,
            RemoteRequesterUserId);
    }

    auto *ExistingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteRequesterUserId);
    if (ExistingRequest == nullptr)
    {
        // There is no friend request from the target user.
        return FRejectFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The friend request was not found (the target user has not sent you a friend request).")),
            LocalUserId,
            RemoteRequesterUserId);
    }

    switch (ExistingRequest->Mode)
    {
    case ESerializedPendingFriendRequestMode::OutboundPendingSend:
    case ESerializedPendingFriendRequestMode::__NoLongerUsed_OutboundPendingReceive:
    case ESerializedPendingFriendRequestMode::OutboundPendingResponse: {
        // We've sent this person a friend request - we don't have a friend request from them.
        return FRejectFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The inbound friend request was not found (you have an outbound friend request to this user "
                     "already).")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    case ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion: {
        // We don't need to do anything with a deletion.
        return FRejectFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The inbound friend request was not found (you have recently deleted this friend).")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    case ESerializedPendingFriendRequestMode::InboundPendingResponse: {
        // Reject the friend request and then queue the reject message immediately.
        ExistingRequest->Mode = ESerializedPendingFriendRequestMode::InboundPendingSendRejection;
        State->bDirty = true;

        // Save the pending rejection before we attempt to send.
        return (this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh) +
                [this, LocalUserId, RemoteRequesterUserId, State](const FError &SaveErrorCode) {
                    if (!SaveErrorCode.WasSuccessful())
                    {
                        // We couldn't save it.
                        UE_LOG(
                            LogRedpointEOSFriends,
                            Error,
                            TEXT("Unable to save cross-platform friends database to reject friend request."));
                        return MakeFulfilledPromise<FError>(Errors::UnexpectedError(
                                                                ANSI_TO_TCHAR(__FUNCTION__),
                                                                TEXT("Unable to save cross-platform friends "
                                                                     "database to reject friend request.")))
                            .GetFuture();
                    }

                    // We've saved it so we can now safely send the rejection notification.
                    return this->QueueSendFriendRejectionAndWait(LocalUserId, RemoteRequesterUserId)
                        .Next([](const FError &) {
                            // It doesn't matter if sending the reject failed, it'll continue to be in the
                            // queue even if it did.
                            return Errors::Success();
                        });
                })
            .Next([LocalUserId, RemoteRequesterUserId](FError OperationErrorCode) {
                return FRejectFriendRequestResult(OperationErrorCode, LocalUserId, RemoteRequesterUserId);
            });
    }
    case ESerializedPendingFriendRequestMode::InboundPendingSendAcceptance: {
        // This friend request has already been accepted.
        return FRejectFriendRequestResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The inbound friend request was not found (it has already been accepted).")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    case ESerializedPendingFriendRequestMode::InboundPendingSendRejection: {
        // This friend request has already been rejected.
        return FRejectFriendRequestResult::Future(Errors::Success(), LocalUserId, RemoteRequesterUserId);
    }
    default: {
        checkf(false, TEXT("Friend request in an invalid state for SendFriendRequest."));
        return FRejectFriendRequestResult::Future(
            Errors::UnexpectedError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The existing friend request was in an invalid state that AcceptFriendRequest can't handle.")),
            LocalUserId,
            RemoteRequesterUserId);
    }
    }
}

bool FFriendSystemImpl::IsFriendDeletable(const FIdentityUserId &LocalUserId, const FIdentityUserId &RemoteFriendUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        UE_LOG(
            LogRedpointEOSFriends,
            Error,
            TEXT("%s"),
            *Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in."))
                 .ToLogString());
        return false;
    }
    auto &State = *StatePtr;

    if (State->FriendDatabase->Friends.Contains(RemoteFriendUserId))
    {
        return true;
    }

    auto *PendingRequest = State->FriendDatabase->PendingFriendRequests.Find(RemoteFriendUserId);
    if (PendingRequest != nullptr)
    {
        if (PendingRequest->Mode == ESerializedPendingFriendRequestMode::OutboundPendingSend ||
            PendingRequest->Mode == ESerializedPendingFriendRequestMode::__NoLongerUsed_OutboundPendingReceive ||
            PendingRequest->Mode == ESerializedPendingFriendRequestMode::OutboundPendingResponse)
        {
            // Allow cancellation of outbound requests we haven't sent yet, or those we're waiting on the remote user to
            // respond to.
            //
            // @note: The latter needs us to send a deletion message as normal.
            return true;
        }
    }

    return false;
}

TFuture<IFriendSystem::FDeleteFriendResult> FFriendSystemImpl::DeleteFriend(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteFriendUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return FDeleteFriendResult::Future(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")),
            LocalUserId,
            RemoteFriendUserId);
    }
    auto &State = *StatePtr;

    auto *ExistingFriend = State->FriendDatabase->Friends.Find(RemoteFriendUserId);
    if (ExistingFriend == nullptr)
    {
        // The user isn't a friend.
        return FDeleteFriendResult::Future(
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The friend does not exist (or they aren't deletable).")),
            LocalUserId,
            RemoteFriendUserId);
    }

    // Remove any existing request for this friend.
    FFriendSystemImpl::RemovePendingFriendRequest(
        State,
        RemoteFriendUserId,
        TEXT("DeleteFriend called and we're removing prior to adding the new pending friend request entry."));

    // Remove the friend.
    State->FriendDatabase->Friends.Remove(RemoteFriendUserId);

    // Schedule a removal notification.
    {
        FSerializedPendingFriendRequest NewRequest = {};
        NewRequest.ProductUserId = GetUserIdString(RemoteFriendUserId);
        NewRequest.Mode = ESerializedPendingFriendRequestMode::OutboundPendingSendDeletion;
        NewRequest.NextAttempt = FDateTime::MinValue();
        FFriendSystemImpl::AddPendingFriendRequest(
            State,
            RemoteFriendUserId,
            MoveTemp(NewRequest),
            TEXT("DeleteFriend was called."));
    }
    State->bDirty = true;

    // Save the pending deletion before we attempt to send.
    return this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh) +
           [this, LocalUserId, RemoteFriendUserId, State](FError SaveErrorCode) {
               if (!SaveErrorCode.WasSuccessful())
               {
                   // We couldn't save it.
                   UE_LOG(
                       LogRedpointEOSFriends,
                       Error,
                       TEXT("Unable to save cross-platform friends database to delete friend request."));
                   return FDeleteFriendResult::Future(
                       Errors::UnexpectedError(
                           ANSI_TO_TCHAR(__FUNCTION__),
                           TEXT("Unable to save cross-platform friends database to delete friend request.")),
                       LocalUserId,
                       RemoteFriendUserId);
               }

               // We've saved it so we can now safely send the deletion notification.
               return this->QueueSendFriendDeletionAndWait(LocalUserId, RemoteFriendUserId)
                   .Next([LocalUserId, RemoteFriendUserId](const FError &) {
                       // It doesn't matter if sending the delete failed, it'll continue to be in the queue
                       // even if it did.
                       return FDeleteFriendResult(Errors::Success(), LocalUserId, RemoteFriendUserId);
                   });
           };
}

FError FFriendSystemImpl::GetRecentPlayers(
    const FIdentityUserId &LocalUserId,
    TAccountIdMap<TTuple<FUserInfoRef, FDateTime>> &OutRecentPlayers)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in."));
    }
    auto &State = *StatePtr;

    OutRecentPlayers = State->RecentPlayers;
    return Errors::Success();
}

void FFriendSystemImpl::RecordRecentPlayer(const FIdentityUserId &RecentPlayerId)
{
    // Make sure this recent player account ID is valid.
    if (!RecentPlayerId.IsValid() || !IsProductUserId(RecentPlayerId))
    {
        return;
    }
    auto AccountIdString = GetUserIdString(RecentPlayerId);
    if (AccountIdString.IsEmpty())
    {
        return;
    }

    auto UserCacheSystem = this->PlatformHandle->GetSystem<IUserCacheSystem>();

    // When a recent player is seen, they're seen for all signed in users.
    for (auto &KV : this->LocalUserFriendStates)
    {
        // Update or add the recent player in cross-platform friend database.
        auto *Existing = KV.Value->FriendDatabase->RecentPlayers.Find(RecentPlayerId);
        if (Existing != nullptr)
        {
            Existing->LastSeen = FDateTime::UtcNow();
        }
        else
        {
            KV.Value->FriendDatabase->RecentPlayers.Add(
                RecentPlayerId,
                FSerializedRecentPlayer{AccountIdString, FDateTime::UtcNow()});
        }

        // Update or add the recent player for in-memory state.
        auto *ExistingInMemory = KV.Value->RecentPlayers.Find(RecentPlayerId);
        if (ExistingInMemory != nullptr)
        {
            ExistingInMemory->Get<1>() = FDateTime::UtcNow();
        }
        else
        {
            KV.Value->RecentPlayers.Add(
                RecentPlayerId,
                TTuple<FUserInfoRef, FDateTime>(
                    UserCacheSystem->GetUnresolvedUser(KV.Key, RecentPlayerId),
                    FDateTime::UtcNow()));
        }

        // Flag as dirty, but don't schedule a save. We periodically schedule saves for dirty databases, and we
        // might be calling RecordRecentPlayer very frequently so we don't want to queue up a save for each call.
        KV.Value->bDirty = true;
    }
}

FError FFriendSystemImpl::GetBlockedPlayers(
    const FIdentityUserId &LocalUserId,
    TAccountIdMap<FUserInfoPtr> &OutBlockedPlayers)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in."));
    }
    auto &State = *StatePtr;

    OutBlockedPlayers = State->BlockedPlayers;
    return Errors::Success();
}

TFuture<IFriendSystem::FBlockPlayerResult> FFriendSystemImpl::BlockPlayer(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return FBlockPlayerResult::Future(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")),
            LocalUserId,
            RemoteUserId);
    }
    auto &State = *StatePtr;

    if (State->FriendDatabase->BlockedUsers.Contains(RemoteUserId))
    {
        // The user is already blocked.
        return FBlockPlayerResult::Future(Errors::Success(), LocalUserId, RemoteUserId);
    }

    State->FriendDatabase->BlockedUsers.Add(RemoteUserId).ProductUserId = GetUserIdString(RemoteUserId);
    State->bDirty = true;

    auto UserCacheSystem = this->PlatformHandle->GetSystem<IUserCacheSystem>();
    State->BlockedPlayers.Add(RemoteUserId, UserCacheSystem->GetUnresolvedUser(LocalUserId, RemoteUserId));

    return this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
        .Next([LocalUserId, RemoteUserId](FError ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Warning,
                    TEXT(
                        "Immediate save and refresh of friends database upon BlockPlayer operation failed. Further "
                        "attempts to save the cross-platform friends database will happen in the background. The error "
                        "was: %s"),
                    *ErrorCode.ToLogString());
            }
            return FBlockPlayerResult(Errors::Success(), LocalUserId, RemoteUserId);
        });
}

TFuture<IFriendSystem::FUnblockPlayerResult> FFriendSystemImpl::UnblockPlayer(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return FUnblockPlayerResult::Future(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")),
            LocalUserId,
            RemoteUserId);
    }
    auto &State = *StatePtr;

    if (!State->FriendDatabase->BlockedUsers.Contains(RemoteUserId))
    {
        // The user is already unblocked.
        return FUnblockPlayerResult::Future(Errors::Success(), LocalUserId, RemoteUserId);
    }

    State->FriendDatabase->BlockedUsers.Remove(RemoteUserId);
    State->bDirty = true;

    State->BlockedPlayers.Remove(RemoteUserId);

    return this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
        .Next([LocalUserId, RemoteUserId](FError ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Warning,
                    TEXT(
                        "Immediate save and refresh of friends database upon UnblockPlayer operation failed. Further "
                        "attempts to save the cross-platform friends database will happen in the background. The error "
                        "was: %s"),
                    *ErrorCode.ToLogString());
            }
            return FUnblockPlayerResult(Errors::Success(), LocalUserId, RemoteUserId);
        });
}

TFuture<IFriendSystem::FSetFriendAliasResult> FFriendSystemImpl::SetFriendAlias(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId,
    const FString &FriendAlias)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return FSetFriendAliasResult::Future(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")),
            LocalUserId,
            RemoteUserId);
    }
    auto &State = *StatePtr;

    FString *ExistingAlias = State->FriendDatabase->FriendAliases.Find(RemoteUserId);
    if (ExistingAlias != nullptr && *ExistingAlias == FriendAlias)
    {
        return FSetFriendAliasResult::Future(Errors::Success(), LocalUserId, RemoteUserId);
    }

    State->FriendDatabase->FriendAliases.Add(RemoteUserId, FriendAlias);
    State->bDirty = true;

    return this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
        .Next([LocalUserId, RemoteUserId](FError ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Warning,
                    TEXT(
                        "Immediate save and refresh of friends database upon SetFriendAlias operation failed. Further "
                        "attempts to save the cross-platform friends database will happen in the background. The error "
                        "was: %s"),
                    *ErrorCode.ToLogString());
            }
            return FSetFriendAliasResult(Errors::Success(), LocalUserId, RemoteUserId);
        });
}

TFuture<IFriendSystem::FDeleteFriendAliasResult> FFriendSystemImpl::DeleteFriendAlias(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return FDeleteFriendAliasResult::Future(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user ID is not signed in.")),
            LocalUserId,
            RemoteUserId);
    }
    auto &State = *StatePtr;

    if (!State->FriendDatabase->FriendAliases.Contains(RemoteUserId))
    {
        return FDeleteFriendAliasResult::Future(Errors::Success(), LocalUserId, RemoteUserId);
    }

    State->FriendDatabase->FriendAliases.Remove(RemoteUserId);
    State->bDirty = true;

    return this->Queue(LocalUserId, EOperationType::Save | EOperationType::Refresh)
        .Next([LocalUserId, RemoteUserId](FError ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogRedpointEOSFriends,
                    Warning,
                    TEXT(
                        "Immediate save and refresh of friends database upon DeleteFriendAlias operation failed. "
                        "Further "
                        "attempts to save the cross-platform friends database will happen in the background. The error "
                        "was: %s"),
                    *ErrorCode.ToLogString());
            }
            return FDeleteFriendAliasResult(Errors::Success(), LocalUserId, RemoteUserId);
        });
}

TOptional<FString> FFriendSystemImpl::GetFriendAlias(
    const FIdentityUserId &LocalUserId,
    const FIdentityUserId &RemoteUserId)
{
    auto *StatePtr = this->LocalUserFriendStates.Find(LocalUserId);
    if (StatePtr == nullptr)
    {
        return TOptional<FString>();
    }
    auto &State = *StatePtr;

    FString *ExistingAlias = State->FriendDatabase->FriendAliases.Find(RemoteUserId);
    if (ExistingAlias == nullptr)
    {
        return TOptional<FString>();
    }

    return *ExistingAlias;
}
}

#endif

REDPOINT_EOS_CODE_GUARD_END()