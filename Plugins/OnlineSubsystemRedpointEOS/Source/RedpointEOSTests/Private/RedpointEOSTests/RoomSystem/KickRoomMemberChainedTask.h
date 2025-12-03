// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomLeaveReason.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

namespace REDPOINT_EOS_FILE_NS_ID(2325888234, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FKickRoomMemberChainedTask : public FChainedTask, public TSharedFromThis<FKickRoomMemberChainedTask>
{
private:
    int32 ActorInstanceId;
    int32 TargetInstanceId;
    TSharedRef<FRoomIdPtr> ExpectedRoomId;
    bool bGotLeaveCallback;
    FDelegateHandle RoomLeftEventHandle;
    FDelegateHandle RoomMemberLeftEventHandle;
    bool bSawRoomLeftEvent;
    bool bSawRoomMemberLeftEvent;
    bool bDoubleCall;

public:
    FKickRoomMemberChainedTask(
        int32 InActorInstanceId,
        int32 InTargetInstanceId,
        TSharedRef<FRoomIdPtr> InExpectedRoomId,
        bool bInDoubleCall = false);
    UE_NONCOPYABLE(FKickRoomMemberChainedTask);
    virtual ~FKickRoomMemberChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);

private:
    void OnKickMemberComplete(const FError &ErrorCode, FChainedTaskContextRef Context, FOnComplete OnDone);

    bool OnKickMemberCompleteWaitForEvents(
        float DeltaSeconds,
        FChainedTaskContextRef Context,
        FOnComplete OnDone,
        FDateTime UtcExpiry);

    void OnRoomLeftEvent(
        const FRoomEventContext &EventContext,
        const FRoomRef &Room,
        ERoomLeaveReason Reason,
        FChainedTaskContextRef Context);

    void OnRoomMemberLeftEvent(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        ERoomLeaveReason LeaveReason,
        FChainedTaskContextRef Context);

    void OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(2325888234, Redpoint::EOS::Tests::RoomSystem, FKickRoomMemberChainedTask)
}