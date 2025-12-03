// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/EnsurePublicMemberListChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureSearchState.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"

namespace REDPOINT_EOS_FILE_NS_ID(147717220, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;

FEnsurePublicMemberListChainedTask::FEnsurePublicMemberListChainedTask(
    int32 InSearchingInstanceId,
    TSet<int32> InOnlyPresentInstanceIds,
    TSharedRef<FRoomIdPtr> InRoomId)
    : SearchingInstanceId(InSearchingInstanceId)
    , OnlyPresentInstanceIds(InOnlyPresentInstanceIds)
    , RoomId(InRoomId)
    , StartTimestamp()
{
}

void FEnsurePublicMemberListChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    this->StartTimestamp = FDateTime::UtcNow();
    this->StartSearch(Context, OnDone, 15);
}

void FEnsurePublicMemberListChainedTask::StartSearch(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone,
    int AttemptsRemaining)
{
    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->SearchingInstanceId)).GetRoomSystem();

    auto SearchRoomsRequest = FSearchRoomsRequest(
        GetAccountId(Context->User(this->SearchingInstanceId)),
        NAME_Lobby,
        NAME_AutomationTests,
        this->RoomId.Get().ToSharedRef());
    RoomSystem->SearchRooms(
        SearchRoomsRequest,
        IRoomSystem::FOnSearchRoomsComplete::CreateSP(
            this,
            &FEnsurePublicMemberListChainedTask::OnSearchComplete,
            Context,
            OnDone,
            AttemptsRemaining));
}

void FEnsurePublicMemberListChainedTask::OnSearchComplete(
    const FError &ErrorCode,
    const TArray<FRoomSearchResultRef> &RoomSearchResults,
    FChainedTaskContextRef Context,
    FOnComplete OnDone,
    int AttemptsRemaining)
{
    if (!Context->Assert().TestTrue(
            TEXT("Expected search rooms for EnsurePublicMemberList to succeed!"),
            ErrorCode.WasSuccessful()) ||
        !Context->Assert().TestTrue(
            TEXT("Expected to have exactly one search result for EnsurePublicMemberList!"),
            RoomSearchResults.Num() == 1))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    auto RoomSearchResult = RoomSearchResults[0];
    auto PublicMemberListRoomFeatureSearchState =
        RoomSearchResult->GetRoomFeatureSearchState<IPublicMemberListRoomFeatureSearchState>();
    if (!Context->Assert().TestTrue(
            TEXT("Expected search result to have PublicMemberListRoomFeature"),
            PublicMemberListRoomFeatureSearchState.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    bool bFinalAttempt = AttemptsRemaining <= 0;
    bool bFoundUnexpected = false;
    TSet<FRoomUserId> ExpectedUserIds;
    for (const auto &InstanceId : this->OnlyPresentInstanceIds)
    {
        ExpectedUserIds.Add(GetAccountId(Context->User(InstanceId)));
    }
    for (const auto &SearchMemberId : PublicMemberListRoomFeatureSearchState->CurrentMembers())
    {
        if (ExpectedUserIds.Contains(SearchMemberId))
        {
            ExpectedUserIds.Remove(SearchMemberId);
        }
        else
        {
            if (bFinalAttempt)
            {
                Context->Assert().TestFalse(
                    FString::Printf(
                        TEXT("Did not expect user '%s' to be present in the search state's member list."),
                        *GetUserIdString(SearchMemberId)),
                    true);
            }
            bFoundUnexpected = true;
        }
    }
    bool bSucceeded = !bFoundUnexpected && ExpectedUserIds.Num() == 0;

    if (bSucceeded)
    {
        FTimespan ConsistencyTime = FDateTime::UtcNow() - this->StartTimestamp;
        Context->Assert().AddInfo(FString::Printf(
            TEXT("Public member list took %f seconds to become consistent with expected state."),
            ConsistencyTime.GetTotalSeconds()));
        OnDone.ExecuteIfBound(true);
        return;
    }
    else if (AttemptsRemaining > 0)
    {
        FTSTicker::GetCoreTicker().AddTicker(
            TEXT("EnsurePublicMemberListChainedTask_Retry"),
            1.0f,
            [this, Context, OnDone, AttemptsRemaining](float DeltaSeconds) {
                this->StartSearch(Context, OnDone, AttemptsRemaining - 1);
                return false;
            });
        return;
    }
    else
    {
        Context->Assert().TestFalse(
            TEXT("Found unexpected member IDs in search state's public member list."),
            bFoundUnexpected);
        for (const auto &RemainingUserId : ExpectedUserIds)
        {
            Context->Assert().TestTrue(
                FString::Printf(
                    TEXT("Expected user ID '%s' to be present in the search state's member list."),
                    *GetUserIdString(RemainingUserId)),
                false);
        }
        Redpoint::EOS::Core::Id::DumpAccountIdRegistry();
        OnDone.ExecuteIfBound(false);
        return;
    }
}

}