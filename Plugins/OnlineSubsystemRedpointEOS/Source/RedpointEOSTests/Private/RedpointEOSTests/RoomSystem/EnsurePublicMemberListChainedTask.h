// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomSearchResult.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

namespace REDPOINT_EOS_FILE_NS_ID(147717220, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FEnsurePublicMemberListChainedTask : public FChainedTask,
                                           public TSharedFromThis<FEnsurePublicMemberListChainedTask>
{
private:
    int32 SearchingInstanceId;
    TSet<int32> OnlyPresentInstanceIds;
    TSharedRef<FRoomIdPtr> RoomId;
    FDateTime StartTimestamp;

public:
    FEnsurePublicMemberListChainedTask(
        int32 InSearchingInstanceId,
        TSet<int32> InOnlyPresentInstanceIds,
        TSharedRef<FRoomIdPtr> InRoomId);
    UE_NONCOPYABLE(FEnsurePublicMemberListChainedTask);
    virtual ~FEnsurePublicMemberListChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);

private:
    void StartSearch(const FChainedTaskContextRef &Context, const FOnComplete &OnDone, int AttemptsRemaining);
    void OnSearchComplete(
        const FError &ErrorCode,
        const TArray<FRoomSearchResultRef> &RoomSearchResults,
        FChainedTaskContextRef Context,
        FOnComplete OnDone,
        int AttemptsRemaining);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(147717220, Redpoint::EOS::Tests::RoomSystem, FEnsurePublicMemberListChainedTask)
}