// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"
#include "RedpointEOSCore/Utils/PeriodicTimer.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSIdentity/IdentityUserSlot.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPresence/PresenceMessageGlobalConnection.h"
#include "RedpointEOSPresence/PresenceSystemRemoteUserState.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2345566497, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;

typedef TMulticastDelegate<void(uint32 JustProcessedPresenceUpdate, const FError &ErrorCode)>
    FOnDesiredPresenceProcessed;

extern const FName LobbyProvider;
extern const FName CrossPlatformPresenceNamespace;

class FPresenceSystemLocalUserState : public TSharedFromThis<FPresenceSystemLocalUserState>
{
private:
    static FString GenerateFriendCode();

    FRoomPtr CurrentPresenceRoom;
    FDelegateHandle RoomSystemOnRoomLeftHandle;

public:
    FPlatformHandle PlatformHandle;
    const FIdentityUserId LocalUserId;
    const FIdentityUserSlot LocalUserSlot;
    TSharedRef<FExclusiveAsyncOperationQueue> OperationQueue;
    FPresenceUserState CurrentPresence;
    FPresenceUserState DesiredPresence;
    TMap<FIdentityUserId, FPresenceSystemRemoteUserState> RemoteUsers;
    TMap<TTuple<EOS_EExternalAccountType, FString>, FIdentityUserId> ExternalAccountIdsToRemoteUserIds;
    FString FriendCode;
    uint32 ProcessedPresenceUpdate;
    uint32 LastScheduledPresenceUpdate;
    FOnDesiredPresenceProcessed OnDesiredPresenceProcessedDelegate;
    FPresenceMessageGlobalConnection::FOnRemoteConnectionReceived OnRemoteConnectionReceivedDelegate;
    TMap<FIdentityUserId, TSharedPtr<FPresenceMessageGlobalConnection>> OpenConnections;
    TMap<TSharedRef<IRuntimePlatformPresenceService>, FDelegateHandle> OnNonUnifiedFriendPresenceChangedDelegateHandles;
    TDelegate<void(const FIdentityUserId &RemoteUserId, const FPresenceUserState &NewPresence)>
        OnNonUnifiedFriendPresenceChangeRequiresBroadcast;
    TSharedRef<FPeriodicTimer> RecreateAfterConnectionLossTimer;
    bool bDisconnectedFromPresenceRoom;
    FSimpleDelegate OnDisconnectionStateChanged;

    FPresenceSystemLocalUserState(
        const FPlatformHandle &InPlatformHandle,
        const FIdentityUserId &InLocalUserId,
        const FIdentityUserSlot &InLocalUserSlot);
    UE_NONCOPYABLE(FPresenceSystemLocalUserState);

    /**
     * The delegate type used to signal completion of the Init function call.
     */
    typedef TDelegate<void(const FError &ErrorCode)> FOnInitComplete;

    /**
     * Initialize the underlying cross-platform presence lobby in the room system for this local user's state.
     */
    void Init(const FOnInitComplete &InCompletionDelegate);

    /**
     * The delegate type used to signal completion of the Shutdown function call.
     */
    typedef TDelegate<void(const FError &ErrorCode)> FOnShutdownComplete;

    /**
     * Shutdown the underlying cross-platform presence lobby in the room system for this local user's state.
     */
    void Shutdown(const FOnShutdownComplete &InCompletionDelegate);

    /**
     * Update the cross-platform presence lobby with the desired state.
     */
    void UpdatePresenceRoomWithDesiredSnapshot(
        const FPresenceUserState &DesiredState,
        const FAsyncTaskGraph::FOnComplete &OnTaskComplete);

    /**
     * Return the room ID of the current cross-platform presence room.
     */
    FRoomIdPtr GetCurrentPresenceRoomId() const;

private:
    void OnRoomMemberJoined(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser);
    void OnRoomMemberLeft(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        ERoomLeaveReason Reason);
    void OnRemoteConnectionReceived(const TSharedRef<IPresenceMessageConnection> &NewConnection);
    void OnRoomLeft(const FRoomEventContext &EventContext, const FRoomRef &Room, ERoomLeaveReason Reason);

    void UpdatePresenceRoomWithDesiredSnapshotOutsideQueue(
        const FPresenceUserState &DesiredState,
        const FAsyncTaskGraph::FOnComplete &OnTaskComplete);

    void RecreatePresenceRoomIfLost();
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(2345566497, Redpoint::EOS::Presence, FOnDesiredPresenceProcessed)
REDPOINT_EOS_FILE_NS_EXPORT(2345566497, Redpoint::EOS::Presence, FPresenceSystemLocalUserState)
REDPOINT_EOS_FILE_NS_EXPORT(2345566497, Redpoint::EOS::Presence, LobbyProvider)
REDPOINT_EOS_FILE_NS_EXPORT(2345566497, Redpoint::EOS::Presence, CrossPlatformPresenceNamespace)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()