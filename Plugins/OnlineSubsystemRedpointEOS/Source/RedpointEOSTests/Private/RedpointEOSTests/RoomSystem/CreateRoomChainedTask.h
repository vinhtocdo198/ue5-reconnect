// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/CreateRoomRequest.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomJoinReason.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

namespace REDPOINT_EOS_FILE_NS_ID(2771099875, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Rooms::Requests;

class FCreateRoomChainedTask : public FChainedTask, public TSharedFromThis<FCreateRoomChainedTask>
{
private:
    int32 OwnerInstanceId;
    FGuid SearchGuid;
    TSharedRef<FRoomIdPtr> ExpectedRoomId;
    TDelegate<void(FCreateRoomRequest &Request)> CreateRoomRequestConfiguration;
    bool bGotCreateCallback;
    FDelegateHandle RoomJoinedEventHandle;
    bool bSawRoomJoinedEvent;

public:
    FCreateRoomChainedTask(
        int32 InOwnerInstanceId,
        FGuid InSearchGuid,
        TSharedRef<FRoomIdPtr> InExpectedRoomId,
        TDelegate<void(FCreateRoomRequest &Request)> InCreateRoomRequestConfiguration);
    UE_NONCOPYABLE(FCreateRoomChainedTask);
    virtual ~FCreateRoomChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);

private:
    void OnCreateComplete(
        const FError &ErrorCode,
        const FRoomPtr &Room,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);

    void OnRoomJoinedEvent(
        const FRoomEventContext &EventContext,
        const FRoomRef &Room,
        ERoomJoinReason Reason,
        FChainedTaskContextRef Context);

    void OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(2771099875, Redpoint::EOS::Tests::RoomSystem, FCreateRoomChainedTask)
}