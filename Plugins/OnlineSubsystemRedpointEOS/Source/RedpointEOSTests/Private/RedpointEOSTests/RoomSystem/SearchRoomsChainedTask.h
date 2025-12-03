// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/SearchRoomsRequest.h"
#include "RedpointEOSRooms/RoomSearchResult.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2719536625, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;

class FSearchRoomsChainedTask : public FChainedTask, public TSharedFromThis<FSearchRoomsChainedTask>
{
private:
    int32 SearcherInstanceId;
    TMap<FString, FRoomAttributeConstraint> AttributeConstraints;
    TSharedPtr<FRoomIdPtr> SearchByRoomId;
    TDelegate<void(const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &SearchResults)>
        ValidateResults;
    bool bExpectsNoSearchResults;

public:
    FSearchRoomsChainedTask(
        int32 InSearcherInstanceId,
        const TMap<FString, FRoomAttributeConstraint> &InAttributeConstraints,
        TDelegate<void(const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &SearchResults)>
            InValidateResults,
        bool bInExpectsNoSearchResults = false);
    FSearchRoomsChainedTask(
        int32 InSearcherInstanceId,
        const TSharedPtr<FRoomIdPtr> &InTargetRoomId,
        TDelegate<void(const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &SearchResults)>
            InValidateResults,
        bool bInExpectsNoSearchResults = false);
    UE_NONCOPYABLE(FSearchRoomsChainedTask);
    virtual ~FSearchRoomsChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnSearchComplete(
        const FError &ErrorCode,
        const TArray<FRoomSearchResultRef> &RoomSearchResults,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(2719536625, Redpoint::EOS::Tests::RoomSystem, FSearchRoomsChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()