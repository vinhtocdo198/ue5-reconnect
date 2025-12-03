// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

namespace REDPOINT_EOS_FILE_NS_ID(4232089186, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FEnsureRoomMembersChainedTask : public FChainedTask, public TSharedFromThis<FEnsureRoomMembersChainedTask>
{
private:
    TSet<int32> PresentInstanceIds;
    TSet<int32> NotPresentInstanceIds;
    TSharedRef<FRoomIdPtr> ExpectedRoomId;

public:
    FEnsureRoomMembersChainedTask(
        TSet<int32> InPresentInstanceIds,
        TSet<int32> InNotPresentInstanceIds,
        TSharedRef<FRoomIdPtr> InExpectedRoomId);
    UE_NONCOPYABLE(FEnsureRoomMembersChainedTask);
    virtual ~FEnsureRoomMembersChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(4232089186, Redpoint::EOS::Tests::RoomSystem, FEnsureRoomMembersChainedTask)
}