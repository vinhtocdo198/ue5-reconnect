// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomEventContext.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomInvite.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

namespace REDPOINT_EOS_FILE_NS_ID(1652404932, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FSendRoomInviteChainedTask : public FChainedTask, public TSharedFromThis<FSendRoomInviteChainedTask>
{
private:
    int32 ActorInstanceId;
    int32 TargetInstanceId;
    TSharedRef<FRoomIdPtr> ExpectedRoomId;
    TSharedRef<FRoomInvitePtr> ReceivedRoomInvite;
    FDelegateHandle RoomInviteReceivedHandle;
    bool bSawRoomInviteReceivedEvent;

public:
    FSendRoomInviteChainedTask(
        int32 InActorInstanceId,
        int32 InTargetInstanceId,
        TSharedRef<FRoomIdPtr> InExpectedRoomId,
        TSharedRef<FRoomInvitePtr> OutReceivedRoomInvite);
    UE_NONCOPYABLE(FSendRoomInviteChainedTask);
    virtual ~FSendRoomInviteChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);

private:
    void OnSendInviteComplete(const FError &ErrorCode, FChainedTaskContextRef Context, FOnComplete OnDone);

    bool OnSendInviteCompleteWaitForEvents(
        float DeltaSeconds,
        FChainedTaskContextRef Context,
        FOnComplete OnDone,
        FDateTime UtcExpiry);

    void OnRoomInviteReceived(
        const FRoomEventContext &EventContext,
        const FRoomInviteRef &IncomingRoomInvite,
        FChainedTaskContextRef Context);

    void OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(1652404932, Redpoint::EOS::Tests::RoomSystem, FSendRoomInviteChainedTask)
}