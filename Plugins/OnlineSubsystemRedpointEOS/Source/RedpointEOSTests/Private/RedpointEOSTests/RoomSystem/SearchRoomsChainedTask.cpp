// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/SearchRoomsChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2719536625, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Tests::RoomSystem;

FSearchRoomsChainedTask::FSearchRoomsChainedTask(
    int32 InSearcherInstanceId,
    const TMap<FString, FRoomAttributeConstraint> &InAttributeConstraints,
    TDelegate<void(const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &SearchResults)>
        InValidateResults,
    bool bInExpectsNoSearchResults)
    : SearcherInstanceId(InSearcherInstanceId)
    , AttributeConstraints(InAttributeConstraints)
    , SearchByRoomId()
    , ValidateResults(InValidateResults)
    , bExpectsNoSearchResults(bInExpectsNoSearchResults)
{
}

FSearchRoomsChainedTask::FSearchRoomsChainedTask(
    int32 InSearcherInstanceId,
    const TSharedPtr<FRoomIdPtr> &InTargetRoomId,
    TDelegate<void(const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &SearchResults)>
        InValidateResults,
    bool bInExpectsNoSearchResults)
    : SearcherInstanceId(InSearcherInstanceId)
    , AttributeConstraints()
    , SearchByRoomId(InTargetRoomId)
    , ValidateResults(InValidateResults)
    , bExpectsNoSearchResults(bInExpectsNoSearchResults)
{
}

void FSearchRoomsChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->SearcherInstanceId)).GetRoomSystem();

    auto Request = this->SearchByRoomId.IsValid() ? FSearchRoomsRequest(
                                                        GetAccountId(Context->User(this->SearcherInstanceId)),
                                                        NAME_Lobby,
                                                        NAME_AutomationTests,
                                                        (*this->SearchByRoomId).ToSharedRef())
                                                  : FSearchRoomsRequest(
                                                        GetAccountId(Context->User(this->SearcherInstanceId)),
                                                        NAME_Lobby,
                                                        NAME_AutomationTests,
                                                        this->AttributeConstraints);

    // @note: If we're not expecting any search results, turn off durability so we don't do eventual consistency
    // retries.
    Request.bDurable = !this->bExpectsNoSearchResults;

    RoomSystem->SearchRooms(
        Request,
        IRoomSystem::FOnSearchRoomsComplete::CreateSP(
            this,
            &FSearchRoomsChainedTask::OnSearchComplete,
            Context,
            OnDone));
}

void FSearchRoomsChainedTask::OnSearchComplete(
    const FError &ErrorCode,
    const TArray<FRoomSearchResultRef> &RoomSearchResults,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    Context->Assert().TestTrue(
        FString::Printf(
            TEXT("Expected SearchRooms for lobby provider to succeed, but got error '%s'."),
            *ErrorCode.ToLogString()),
        ErrorCode.WasSuccessful());

    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->SearcherInstanceId)).GetRoomSystem();

    this->ValidateResults.ExecuteIfBound(Context, RoomSearchResults);

    OnDone.ExecuteIfBound(true);
}

}

REDPOINT_EOS_CODE_GUARD_END()