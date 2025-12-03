// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/UpdateRoomRequest.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomJoinReason.h"
#include "RedpointEOSRooms/RoomUpdateInfo.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

namespace REDPOINT_EOS_FILE_NS_ID(3383242971, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Rooms::Requests;

class FUpdateRoomChainedTask : public FChainedTask, public TSharedFromThis<FUpdateRoomChainedTask>
{
private:
    int32 OwnerInstanceId;
    TArray<int32> JoinedInstanceIds;
    TSharedRef<FRoomIdPtr> ExpectedRoomId;
    TDelegate<void(FUpdateRoomRequest &Request)> UpdateRoomRequestConfiguration;
    TDelegate<void(const FChainedTaskContextRef &Context, const FRoomRef &CurrentRoom)> ValidateRoom;
    bool bGotUpdateCallback;
    TMap<int32, FDelegateHandle> RoomUpdatedEventHandles;
    TMap<int32, bool> bSawRoomUpdatedEvent;

public:
    FUpdateRoomChainedTask(
        int32 InOwnerInstanceId,
        TArray<int32> InJoinedInstanceIds,
        TSharedRef<FRoomIdPtr> InExpectedRoomId,
        TDelegate<void(FUpdateRoomRequest &Request)> InUpdateRoomRequestConfiguration,
        TDelegate<void(const FChainedTaskContextRef &Context, const FRoomRef &CurrentRoom)> InValidateRoom);
    UE_NONCOPYABLE(FUpdateRoomChainedTask);
    virtual ~FUpdateRoomChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);

private:
    void OnUpdateComplete(const FError &ErrorCode, FChainedTaskContextRef Context, FOnComplete OnDone);

    void OnRoomUpdatedEvent(
        const FRoomEventContext &EventContext,
        const FRoomRef &Room,
        const FRoomUpdateInfo &RoomUpdateInfo,
        FChainedTaskContextRef Context,
        int32 InstanceId);

    bool OnUpdateCompleteWaitForEvents(
        float DeltaSeconds,
        FChainedTaskContextRef Context,
        FOnComplete OnDone,
        FDateTime UtcExpiry);

    void OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(3383242971, Redpoint::EOS::Tests::RoomSystem, FUpdateRoomChainedTask)
}