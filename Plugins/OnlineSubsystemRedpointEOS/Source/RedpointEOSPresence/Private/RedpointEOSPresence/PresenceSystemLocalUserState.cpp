// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceSystemLocalUserState.h"

REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()
#include "RedpointEOSCompat/OptionalCompat.h"
REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END()

#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSPresence/Logging.h"
#include "RedpointEOSPresence/PresenceUserStateSerialization.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Requests/CreateRoomRequest.h"
#include "RedpointEOSRooms/RoomSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2345566497, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Requests;

const FName LobbyProvider = FName(TEXT("Lobby"));
const FName CrossPlatformPresenceNamespace = FName(TEXT("CrossPlatformPresence"));

FString FPresenceSystemLocalUserState::GenerateFriendCode()
{
    static TArray<char> AvailableCharacters = TArray<char>{'B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'M', 'P', 'Q', 'R',
                                                           'T', 'V', 'W', 'X', 'Y', '3', '4', '6', '7', '8', '9'};
    FString FriendCode = TEXT("");
    int DesiredCharacters = 9;
    int SeparatorEvery = 3;
    for (int i = 0; i < DesiredCharacters; i++)
    {
        FriendCode.AppendChar(
            CharCast<TCHAR, ANSICHAR>(AvailableCharacters[FMath::RandRange(0, AvailableCharacters.Num() - 1)]));
        if ((i + 1) % SeparatorEvery == 0 && i < DesiredCharacters - 1)
        {
            FriendCode.AppendChar('-');
        }
    }
    return FriendCode;
}

FPresenceSystemLocalUserState::FPresenceSystemLocalUserState(
    const FPlatformHandle &InPlatformHandle,
    const FIdentityUserId &InLocalUserId,
    const FIdentityUserSlot &InLocalUserSlot)
    : CurrentPresenceRoom()
    , RoomSystemOnRoomLeftHandle()
    , PlatformHandle(InPlatformHandle)
    , LocalUserId(InLocalUserId)
    , LocalUserSlot(InLocalUserSlot)
    , OperationQueue(MakeShared<FExclusiveAsyncOperationQueue>())
    , CurrentPresence(InLocalUserId)
    , DesiredPresence(InLocalUserId)
    , RemoteUsers()
    , ExternalAccountIdsToRemoteUserIds()
    , FriendCode(GenerateFriendCode())
    , ProcessedPresenceUpdate()
    , LastScheduledPresenceUpdate()
    , OnDesiredPresenceProcessedDelegate()
    , OnRemoteConnectionReceivedDelegate()
    , OpenConnections()
    , OnNonUnifiedFriendPresenceChangedDelegateHandles()
    , OnNonUnifiedFriendPresenceChangeRequiresBroadcast()
    , RecreateAfterConnectionLossTimer(MakeShared<FPeriodicTimer>(10.0f))
    , bDisconnectedFromPresenceRoom(false)
{
}

void FPresenceSystemLocalUserState::Init(const FOnInitComplete &InCompletionDelegate)
{
    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, this->LocalUserSlot);
    for (const auto &Integration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        this->OnNonUnifiedFriendPresenceChangedDelegateHandles.Add(
            Integration->GetPresenceService(),
            Integration->GetPresenceService()
                ->OnNonUnifiedFriendPresenceChanged(CallContext)
                .Add(IRuntimePlatformPresenceService::FOnNonUnifiedFriendPresenceChanged::FDelegate::
                         CreateSPLambda(
                             this,
                             [this, PresenceService = Integration->GetPresenceService()](
                                 int LocalUserNum,
                                 const EOS_EExternalAccountType &FriendAccountType,
                                 const FString &FriendAccountId,
                                 EPresenceUserActivityState NewPresenceActivityState) {
                                 if (LocalUserNum != this->LocalUserSlot)
                                 {
                                     return;
                                 }

                                 auto *RemoteUserId = this->ExternalAccountIdsToRemoteUserIds.Find(
                                     TTuple<EOS_EExternalAccountType, FString>(FriendAccountType, FriendAccountId));
                                 if (RemoteUserId == nullptr)
                                 {
                                     return;
                                 }

                                 auto *RemoteUser = this->RemoteUsers.Find(*RemoteUserId);
                                 if (RemoteUser == nullptr)
                                 {
                                     return;
                                 }

                                 if (!RemoteUser->PartialPresence.IsSet())
                                 {
                                     // We haven't fetched initial data yet. Leave it to that code to pull the
                                     // information rather than this event.
                                     return;
                                 }

                                 auto *Value = RemoteUser->PartialRuntimeActivityState.Find(PresenceService);
                                 if (Value == nullptr || *Value == NewPresenceActivityState)
                                 {
                                     // This value is already up-to-date.
                                     return;
                                 }

                                 *Value = NewPresenceActivityState;
                                 auto OldHash = GetTypeHash(RemoteUser->PartialPresence);
                                 RemoteUser->RecomputePartialPresence();

                                 if (OldHash != GetTypeHash(RemoteUser->PartialPresence) &&
                                     !RemoteUser->IsBestPresenceFullPresence())
                                 {
                                     this->OnNonUnifiedFriendPresenceChangeRequiresBroadcast.ExecuteIfBound(
                                         RemoteUser->UserId,
                                         *RemoteUser->PartialPresence);
                                 }
                             })));
    }

    this->OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
        this,
        [this, CallContext, InCompletionDelegate](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
            checkf(
                !this->CurrentPresenceRoom.IsValid(),
                TEXT("FPresenceSystemLocalUserState::Init called multiple times!"));

            auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();

            // Register event on the room system so that we can detect if we ever leave our own presence
            // room (which can happen if the Internet connection is lost).
            this->RoomSystemOnRoomLeftHandle = RoomSystem->OnRoomLeft(CrossPlatformPresenceNamespace)
                                                   .AddSP(this, &FPresenceSystemLocalUserState::OnRoomLeft);

            // Create the minimal presence lobby for this user. We start this off not publicly advertised and use the
            // periodic cross-platform state update to make it publicly advertised after it's created, which allows us
            // to have only one copy of the state-sync logic.
            auto Request = FCreateRoomRequest(
                ERoomScope::User,
                this->LocalUserId,
                LobbyProvider,
                CrossPlatformPresenceNamespace,
                16 /* @note: Allow other players to join for messaging. */);
            Request.Attributes.Add(TEXT("UserId"), GetUserIdString(this->LocalUserId));
            Request.Attributes.Add(TEXT("FriendCode"), FriendCode);
            Request.Attributes.Add(TEXT("IsCrossPlatformPresence"), true);
            RoomSystem->CreateRoom(
                Request,
                IRoomSystem::FOnCreateRoomComplete::CreateSPLambda(
                    this,
                    [this, CallContext, RoomSystem, InCompletionDelegate, InOnDone](FError ErrorCode, FRoomPtr Room) {
                        // Log any error.
                        if (!ErrorCode.WasSuccessful())
                        {
                            if (!GIsAutomationTesting)
                            {
                                UE_LOG(
                                    LogRedpointEOSPresence,
                                    Error,
                                    TEXT("Local user %s could not create cross-platform presence room. We'll try to "
                                         "create the cross-platform presence room later on a future update. The error "
                                         "was: %s"),
                                    *GetUserIdString(this->LocalUserId),
                                    *ErrorCode.ToLogString());
                            }
                        }
                        else
                        {
                            // Register events on the room so that we can detect when other people connect to us for
                            // message connections.
                            Room->OnRoomMemberJoined().AddSP(this, &FPresenceSystemLocalUserState::OnRoomMemberJoined);
                            Room->OnRoomMemberLeft().AddSP(this, &FPresenceSystemLocalUserState::OnRoomMemberLeft);

                            // Otherwise, we've created the room and can proceed as normal.
                            this->CurrentPresenceRoom = Room;

                            UE_LOG(
                                LogRedpointEOSPresence,
                                Verbose,
                                TEXT("Cross-platform presence lobby for user '%s' has ID '%s'."),
                                *GetUserIdString(this->LocalUserId),
                                *Room->GetRoomId()->ToString());
                        }

                        // Regardless of whether the room was created, start the periodic timer that recreates the lobby
                        // if it's missing now.
                        this->RecreateAfterConnectionLossTimer->Start(FPeriodicTimerDelegate::CreateSP(
                            this,
                            &FPresenceSystemLocalUserState::RecreatePresenceRoomIfLost));

                        // Mark this operation as completed.
                        InCompletionDelegate.ExecuteIfBound(Errors::Success());
                        InOnDone.ExecuteIfBound();
                        return;
                    }));
        }));
}

void FPresenceSystemLocalUserState::Shutdown(const FOnShutdownComplete &InCompletionDelegate)
{
    this->RecreateAfterConnectionLossTimer->Stop();

    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, this->LocalUserSlot);
    for (const auto &HandleKV : this->OnNonUnifiedFriendPresenceChangedDelegateHandles)
    {
        HandleKV.Key->OnNonUnifiedFriendPresenceChanged(CallContext).Remove(HandleKV.Value);
    }
    this->OnNonUnifiedFriendPresenceChangedDelegateHandles.Empty();

    this->OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
        this,
        [this, InCompletionDelegate](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
            if (!this->CurrentPresenceRoom.IsValid())
            {
                InCompletionDelegate.ExecuteIfBound(Errors::Success());
                InOnDone.ExecuteIfBound();
                return;
            }

            auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();

            RoomSystem->OnRoomLeft(CrossPlatformPresenceNamespace).Remove(this->RoomSystemOnRoomLeftHandle);

            RoomSystem->LeaveRoom(
                this->CurrentPresenceRoom.ToSharedRef(),
                FLeaveRoomRequest(),
                IRoomSystem::FOnLeaveRoomComplete::CreateSPLambda(
                    this,
                    [this, InOnDone, InCompletionDelegate](FError ErrorCode) {
                        if (!ErrorCode.WasSuccessful())
                        {
                            UE_LOG(
                                LogRedpointEOSPresence,
                                Warning,
                                TEXT("Unable to remove cross-platform presence lobby for local user %s: %s"),
                                *GetUserIdString(this->LocalUserId),
                                *ErrorCode.ToLogString());
                        }
                        else
                        {
                            this->CurrentPresenceRoom.Reset();
                        }

                        InCompletionDelegate.ExecuteIfBound(ErrorCode);
                        InOnDone.ExecuteIfBound();
                        return;
                    }));
        }));
}

void FPresenceSystemLocalUserState::UpdatePresenceRoomWithDesiredSnapshot(
    const FPresenceUserState &DesiredLocalUserStateSnapshot,
    const FAsyncTaskGraph::FOnComplete &OnTaskComplete)
{
    this->OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
        this,
        [this, DesiredLocalUserStateSnapshot, OnTaskComplete](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
            this->UpdatePresenceRoomWithDesiredSnapshotOutsideQueue(
                DesiredLocalUserStateSnapshot,
                FAsyncTaskGraph::FOnComplete::CreateLambda([OnTaskComplete, InOnDone](const FError &ErrorCode) {
                    OnTaskComplete.ExecuteIfBound(ErrorCode);
                    InOnDone.ExecuteIfBound();
                }));
        }));
}

FRoomIdPtr FPresenceSystemLocalUserState::GetCurrentPresenceRoomId() const
{
    if (this->CurrentPresenceRoom.IsValid())
    {
        return this->CurrentPresenceRoom->GetRoomId();
    }
    return nullptr;
}

void FPresenceSystemLocalUserState::OnRoomMemberJoined(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &Room,
    FRoomUserId TargetUser)
{
    // Our open connections list can contain either:
    // - A client connection we've made to someone else's room
    // - A host connection that we've received because they've connected to us
    // We only need one of these for a particular target user, and which one we get depends on who is sending messages
    // first. We never want a duplicate global connection (i.e. we never want to be connected to a target user's room if
    // they've already connected to us).
    if (!this->OpenConnections.Contains(TargetUser))
    {
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Creating global presence message connection in response to user %s joining our presence lobby."),
            *GetUserIdString(TargetUser));

        auto GlobalConnection = MakeSharedWithEvents<FPresenceMessageGlobalConnection>(
            this->PlatformHandle->GetSystem<IRoomSystem>(),
            this->LocalUserId,
            TargetUser,
            Room,
            NAME_None);
        GlobalConnection->OnConnectionClosedDelegate.Add(
            FPresenceMessageGlobalConnection::FOnConnectionClosed::FDelegate::CreateSPLambda(
                this,
                [TargetUser,
                 PresenceConnectionWk = TWeakPtr<FPresenceMessageGlobalConnection>(GlobalConnection),
                 LocalUserStateWk = TWeakPtr<FPresenceSystemLocalUserState>(this->AsShared())]() {
                    auto PresenceConnectionPinned = PresenceConnectionWk.Pin();
                    auto LocalUserStatePinned = LocalUserStateWk.Pin();
                    if (PresenceConnectionPinned.IsValid() && LocalUserStatePinned.IsValid())
                    {
                        auto *CurrentValue = LocalUserStatePinned->OpenConnections.Find(TargetUser);
                        if (CurrentValue != nullptr && *CurrentValue == PresenceConnectionPinned)
                        {
                            LocalUserStatePinned->OpenConnections.Remove(TargetUser);
                        }
                    }
                }));
        GlobalConnection->OnRemoteConnectionReceivedDelegate.AddSP(
            this,
            &FPresenceSystemLocalUserState::OnRemoteConnectionReceived);
        this->OpenConnections.Add(TargetUser, GlobalConnection);
    }
    else
    {
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Not creating global presence message connection in response to user %s joining our presence lobby, "
                 "because we already have an outbound connection to that user."),
            *GetUserIdString(TargetUser));
    }
}

void FPresenceSystemLocalUserState::OnRoomMemberLeft(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &Room,
    FRoomUserId TargetUser,
    ERoomLeaveReason LeaveReason)
{
    // When someone leaves our presence lobby, and we are tracking a connection for them, we know the connection must
    // have been created when they joined the room, as we will not create an outbound connection if one already exists.
    auto *OpenConnectionPtr = this->OpenConnections.Find(TargetUser);
    if (OpenConnectionPtr == nullptr)
    {
        return;
    }

    // @note: This is intentionally *not* a reference, because the memory that OpenConnectionPtr points to will become
    // invalid the moment that OnConnectionClosedDelegate is broadcast and the entry in the map is cleared. Thus we do a
    // copy of a TSharedPtr here to make sure we keep it around.
    auto OpenConnection = *OpenConnectionPtr;

    checkf(
        Room->GetRoomId() == OpenConnection->Room->GetRoomId(),
        TEXT("Expected tracked presence connection to be for the member in the room!"));

    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Remote user '%s' has left our ('%s') presence lobby, so we are closing the global presence connection "
             "that represents them."),
        *GetUserIdString(TargetUser),
        *GetUserIdString(this->LocalUserId))

    OpenConnection->CloseDueToRemoteUserLeaving();

#if DO_CHECK
    // @note: We do not remove the connection from OpenConnections, because CloseDueToRemoteUserLeaving will have
    // broadcast OnConnectionClosedDelegate and the presence system will have removed it by this point. Assert to make
    // sure that happened.
    auto *NewOpenConnectionPtr = this->OpenConnections.Find(TargetUser);
    checkf(
        NewOpenConnectionPtr == nullptr || *NewOpenConnectionPtr != OpenConnection,
        TEXT("Expected CloseDueToRemoteUserLeaving to cause removal of connection entry via "
             "OnConnectionClosedDelegate."));
#endif
}

void FPresenceSystemLocalUserState::OnRemoteConnectionReceived(
    const TSharedRef<IPresenceMessageConnection> &NewConnection)
{
    this->OnRemoteConnectionReceivedDelegate.Broadcast(NewConnection);
}

void FPresenceSystemLocalUserState::OnRoomLeft(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomLeaveReason Reason)
{
    if (!this->CurrentPresenceRoom.IsValid() || !Room->GetRoomId()->IsEqual(this->CurrentPresenceRoom->GetRoomId()))
    {
        return;
    }

    // We are leaving our own presence room.
    UE_LOG(
        LogRedpointEOSPresence,
        Warning,
        TEXT("Lost connection to %s's own cross-platform presence lobby. We will attempt to re-create it on the next "
             "update."),
        *GetUserIdString(this->LocalUserId));
    this->CurrentPresenceRoom.Reset();
    if (Reason == ERoomLeaveReason::Disconnected)
    {
        this->bDisconnectedFromPresenceRoom = true;
        this->OnDisconnectionStateChanged.ExecuteIfBound();
    }
}

void FPresenceSystemLocalUserState::UpdatePresenceRoomWithDesiredSnapshotOutsideQueue(
    const FPresenceUserState &DesiredLocalUserStateSnapshot,
    const FAsyncTaskGraph::FOnComplete &OnTaskComplete)
{
    auto RoomSystem = this->PlatformHandle->GetSystem<IRoomSystem>();

    // If the room exists, update it.
    if (this->CurrentPresenceRoom.IsValid())
    {
        // Compute the room update request.
        auto Request = FUpdateRoomRequest();
        FPresenceUserStateSerialization::SerializeForUpdate(
            Request,
            CurrentPresenceRoom.ToSharedRef(),
            DesiredLocalUserStateSnapshot);

        // Execute the room update.
        RoomSystem->UpdateRoom(
            CurrentPresenceRoom.ToSharedRef(),
            Request,
            IRoomSystem::FOnUpdateRoomComplete::CreateSPLambda(
                this,
                [this, OnTaskComplete](FError ErrorCode) {
                    // Log any error.
                    if (!ErrorCode.WasSuccessful() && !GIsAutomationTesting)
                    {
                        UE_LOG(
                            LogRedpointEOSPresence,
                            Warning,
                            TEXT("Failed to update the cross-platform presence room for local user %s. This "
                                 "update will be rescheduled. The error was: %s"),
                            *GetUserIdString(this->LocalUserId),
                            *ErrorCode.ToLogString());
                    }

                    // Mark this operation as completed.
                    OnTaskComplete.ExecuteIfBound(ErrorCode);
                    return;
                }));
    }
    // Otherwise, the room doesn't exist yet (we either couldn't create it at startup or we lost Internet
    // connectivity). Re-create it.
    else
    {
        UE_LOG(
            LogRedpointEOSPresence,
            Verbose,
            TEXT("Attempting to create cross-platform presence lobby for %s as they do not currently have one."),
            *GetUserIdString(this->LocalUserId));

        // Compute the room create request.
        auto Request = FCreateRoomRequest(
            ERoomScope::User,
            this->LocalUserId,
            LobbyProvider,
            CrossPlatformPresenceNamespace,
            16 /* @note: Allow other players to join for messaging. */);
        Request.Attributes.Add(TEXT("UserId"), GetUserIdString(this->LocalUserId));
        Request.Attributes.Add(TEXT("FriendCode"), FriendCode);
        Request.Attributes.Add(TEXT("IsCrossPlatformPresence"), true);
        FPresenceUserStateSerialization::SerializeForCreate(Request, DesiredLocalUserStateSnapshot);

        // Execute the room create.
        RoomSystem->CreateRoom(
            Request,
            IRoomSystem::FOnCreateRoomComplete::CreateSPLambda(
                this,
                [this, OnTaskComplete](FError ErrorCode, FRoomPtr Room) {
                    // Log any error.
                    if (!ErrorCode.WasSuccessful())
                    {
                        if (!GIsAutomationTesting)
                        {
                            UE_LOG(
                                LogRedpointEOSPresence,
                                Warning,
                                TEXT("Failed to create the cross-platform presence room for local user %s. This "
                                     "create will be rescheduled. The error was: %s"),
                                *GetUserIdString(this->LocalUserId),
                                *ErrorCode.ToLogString());
                        }
                    }
                    else
                    {
                        // Register events on the room so that we can detect when other people connect to us for
                        // message connections.
                        Room->OnRoomMemberJoined().AddSP(this, &FPresenceSystemLocalUserState::OnRoomMemberJoined);
                        Room->OnRoomMemberLeft().AddSP(this, &FPresenceSystemLocalUserState::OnRoomMemberLeft);

                        // Otherwise, we've created the room and can proceed as normal.
                        this->CurrentPresenceRoom = Room;

                        UE_LOG(
                            LogRedpointEOSPresence,
                            Verbose,
                            TEXT("Cross-platform presence lobby for user '%s' has ID '%s'."),
                            *GetUserIdString(this->LocalUserId),
                            *Room->GetRoomId()->ToString());

                        // If we were previously disconnected, let the presence system recompute whether Internet
                        // connectivity has been fully restored.
                        if (this->bDisconnectedFromPresenceRoom)
                        {
                            this->bDisconnectedFromPresenceRoom = false;
                            this->OnDisconnectionStateChanged.ExecuteIfBound();
                        }
                    }

                    // Mark this operation as completed.
                    OnTaskComplete.ExecuteIfBound(ErrorCode);
                    return;
                }));
    }
}

void FPresenceSystemLocalUserState::RecreatePresenceRoomIfLost()
{
    if (this->CurrentPresenceRoom.IsValid())
    {
        return;
    }

    // We do this inside the operation queue so that if a previous "re-create" operation hasn't finished running, we
    // don't try to start another one concurrently.
    this->OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
        this,
        [this](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
            if (!this->CurrentPresenceRoom.IsValid())
            {
                UE_LOG(
                    LogRedpointEOSPresence,
                    Verbose,
                    TEXT(
                        "Attempting to re-create cross-platform presence lobby for user '%s' because it does not exist "
                        "(typically due to a temporary disruption in Internet connectivity)..."),
                    *GetUserIdString(this->LocalUserId));
                this->UpdatePresenceRoomWithDesiredSnapshotOutsideQueue(
                    this->DesiredPresence,
                    FAsyncTaskGraph::FOnComplete::CreateSPLambda(
                        this,
                        [this, InOnDone](const FError &Error) {
                            if (Error.WasSuccessful())
                            {
                                UE_LOG(
                                    LogRedpointEOSPresence,
                                    Verbose,
                                    TEXT("Successfully re-created cross-platform presence lobby for user '%s'."),
                                    *GetUserIdString(this->LocalUserId));
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSPresence,
                                    Error,
                                    TEXT("Failed to re-create cross-platform presence lobby for user '%s': %s"),
                                    *GetUserIdString(this->LocalUserId),
                                    *Error.ToLogString());
                            }
                            InOnDone.ExecuteIfBound();
                        }));
            }
            else
            {
                InOnDone.ExecuteIfBound();
            }
        }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()