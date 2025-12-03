// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomEventContext.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

namespace REDPOINT_EOS_FILE_NS_ID(1399126097, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FPromoteRoomMemberChainedTask : public FChainedTask, public TSharedFromThis<FPromoteRoomMemberChainedTask>
{
private:
    int32 ActorInstanceId;
    int32 TargetInstanceId;
    TSet<int32> ObservingInstanceIds;
    TSharedRef<FRoomIdPtr> ExpectedRoomId;
    TMap<int32, FDelegateHandle> RoomMemberPromotedEventHandles;
    TMap<int32, bool> bSawRoomMemberPromotedEvent;

public:
    FPromoteRoomMemberChainedTask(
        int32 InActorInstanceId,
        int32 InTargetInstanceId,
        TSet<int32> InObservingInstanceIds,
        TSharedRef<FRoomIdPtr> InExpectedRoomId);
    UE_NONCOPYABLE(FPromoteRoomMemberChainedTask);
    virtual ~FPromoteRoomMemberChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);

private:
    void OnPromoteComplete(const FError &ErrorCode, FChainedTaskContextRef Context, FOnComplete OnDone);

    bool OnPromoteCompleteWaitForEvents(
        float DeltaSeconds,
        FChainedTaskContextRef Context,
        FOnComplete OnDone,
        FDateTime UtcExpiry);

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
REDPOINT_EOS_FILE_NS_EXPORT(1399126097, Redpoint::EOS::Tests::RoomSystem, FPromoteRoomMemberChainedTask)
}