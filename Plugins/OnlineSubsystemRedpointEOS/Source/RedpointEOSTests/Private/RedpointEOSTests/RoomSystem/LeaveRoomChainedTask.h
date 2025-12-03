// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomLeaveReason.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

namespace REDPOINT_EOS_FILE_NS_ID(113233376, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FLeaveRoomChainedTask : public FChainedTask, public TSharedFromThis<FLeaveRoomChainedTask>
{
private:
    int32 ActorInstanceId;
    TMap<int32, ERoomLeaveReason> LeavingInstanceIdsAndReasons;
    TSet<int32> PromotedInstanceIds;
    TSharedRef<FRoomIdPtr> ExpectedRoomId;
    bool bGotLeaveCallback;
    TMap<int32, FDelegateHandle> RoomLeftEventHandles;
    TMap<int32, FDelegateHandle> RoomMemberPromotedEventHandles;
    TMap<int32, bool> bSawRoomLeftEvent;
    TMap<int32, bool> bSawRoomMemberPromotedEvent;

public:
    FLeaveRoomChainedTask(
        int32 InActorInstanceId,
        TMap<int32, ERoomLeaveReason> InLeavingInstanceIds,
        TSet<int32> InPromotedInstanceIds,
        TSharedRef<FRoomIdPtr> InExpectedRoomId);
    UE_NONCOPYABLE(FLeaveRoomChainedTask);
    virtual ~FLeaveRoomChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);

private:
    void OnLeaveComplete(const FError &ErrorCode, FChainedTaskContextRef Context, FOnComplete OnDone);

    bool OnLeaveCompleteWaitForEvents(
        float DeltaSeconds,
        FChainedTaskContextRef Context,
        FOnComplete OnDone,
        FDateTime UtcExpiry);

    void OnRoomLeftEvent(
        const FRoomEventContext &EventContext,
        const FRoomRef &Room,
        ERoomLeaveReason Reason,
        FChainedTaskContextRef Context,
        int32 InstanceId,
        ERoomLeaveReason ExpectedLeaveReason);

    void OnRoomMemberPromotedEvent(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        FChainedTaskContextRef Context,
        int32 InstanceId);

    void OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(113233376, Redpoint::EOS::Tests::RoomSystem, FLeaveRoomChainedTask)
}