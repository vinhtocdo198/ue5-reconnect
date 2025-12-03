// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSPresence/PresenceMessageInstancedConnection.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(402271213, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::API;

/**
 * When we establish a connection to a remote user via their presence room, we only have one real connection (we can
 * only join their presence room once). However, we might need to host multiple presence connections to that user. This
 * class acts as a manager that performs all the real work, allowing FPresenceMessageInstancedConnection (which
 * implements IPresenceMessageConnection) to offer an instanced/system-specific view.
 */
class FPresenceMessageGlobalConnection : public TSharedFromThis<FPresenceMessageGlobalConnection>,
                                         public IHasEventRegistration
{
public:
    typedef TMulticastDelegate<void(const TSharedRef<IPresenceMessageConnection> &)> FOnRemoteConnectionReceived;
    typedef TMulticastDelegate<void()> FOnConnectionClosed;

public:
    const TSharedRef<IRoomSystem> RoomSystem;
    const FIdentityUserId LocalUserId;
    const FIdentityUserId TargetUserId;
    const FRoomRef Room;
    TMap<int16, TSharedPtr<FPresenceMessageInstancedConnection>> InstancedConnections;
    TSet<int16> ClosedConnections;
    int64 NextOutboundMessageId;
    int16 NextConnectionId;
    uint32 LastFlushedHash;
    bool bOpen;
    FOnRemoteConnectionReceived OnRemoteConnectionReceivedDelegate;
    FOnConnectionClosed OnConnectionClosedDelegate;

private:
    FDelegateHandle OnRoomLeftDelegateHandle;
    FDelegateHandle OnRoomMemberUpdatedDelegateHandle;
    TSharedRef<FExclusiveAsyncOperationQueue> WorkQueue;
    const FName InitialInterestedSystem;

public:
    FPresenceMessageGlobalConnection(
        const TSharedRef<IRoomSystem> &InRoomSystem,
        const FIdentityUserId &InLocalUserId,
        const FIdentityUserId &InTargetUserId,
        const FRoomRef &InRoom,
        const FName &InInitialInterestedSystem);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FPresenceMessageGlobalConnection);
    virtual ~FPresenceMessageGlobalConnection() override;

    void CloseDueToRemoteUserLeaving();

private:
    void OnRoomLeft(const FRoomEventContext &EventContext, const FRoomRef &Room, ERoomLeaveReason Reason);
    void OnRoomMemberUpdated(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        const FRoomMemberUpdateInfo &RoomMemberUpdateInfo);
    void OnKickRoomMemberOnCloseComplete(
        const FError &ErrorCode,
        TSharedRef<FPresenceMessageGlobalConnection> KeepAlive);
    void OnLeaveRoomOnCloseComplete(const FError &ErrorCode, TSharedRef<FPresenceMessageGlobalConnection> KeepAlive);

public:
    void SendInstancedMessage(
        const TSharedRef<FPresenceMessageInstancedConnection> &InInstancedConnection,
        const FName &InMessageType,
        const TSharedRef<FJsonValue> &InMessage,
        const IPresenceMessageConnection::FOnSendMessageComplete &InCompletionDelegate);
    void CloseInstanced(
        const TSharedRef<FPresenceMessageInstancedConnection> &InInstancedConnection,
        const IPresenceMessageConnection::FOnCloseComplete &InCompletionDelegate);

    TSharedPtr<IPresenceMessageConnection> CreateInstanced(const FName &InInterestedSystem);

private:
    void CloseInternal(bool bRequiresRoomLeaveOrKick);

    void ScheduleWork();

    void ProcessOutboundWork(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone);
    void ProcessInboundWork(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone);
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(402271213, Redpoint::EOS::Presence, FPresenceMessageGlobalConnection)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()