// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"
#include "RedpointEOSTests/Party/SetPartyDataChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/DelayChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1739511371, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_SearchAndJoin,
    "Redpoint.EOS.Party.SearchAndJoin",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_SearchAndJoin::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    FString SearchGuid = FGuid::NewGuid().ToString();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();

    // Create the party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId);

    // Set attributes on the party so we can search.
    Executor->Then<FSetPartyDataChainedTask>(
        0,
        StoredPartyId,
        FSetPartyDataChainedTask::FModifyPartyData::CreateLambda([SearchGuid](FOnlinePartyData &PartyData) {
            PartyData.SetAttribute(TEXT("SearchAttr"), FVariantData(SearchGuid));
        }),
        FSetPartyDataChainedTask::FVerifyPartyData::CreateLambda(
            [](const FChainedTaskContextRef &Context, const FOnlinePartyDataConstPtr &PartyData) {
            }),
        false);

    // Register event handlers.
    Executor->Then<FLambdaChainedTask>([StoredPartyId, SearchGuid, StoredInvite](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        FOnlineLobbySearchQuery Query;
        Query.Filters.Add(FOnlineLobbySearchQueryFilter(
            TEXT("SearchAttr"),
            FVariantData(SearchGuid),
            EOnlineLobbySearchQueryFilterComparator::Equal));
        Query.Limit = 10;
        Context->Assert().TestTrue(
            TEXT("Expected IOnlineLobby::Search to return true."),
            Online::GetLobbyInterface(&Context->Instance(1))
                ->Search(
                    Context->User(1),
                    Query,
                    FOnLobbySearchComplete::CreateLambda([Context, TaskOnDone, StoredInvite](
                                                             const FOnlineError &Error,
                                                             const FUniqueNetId &UserId,
                                                             const TArray<TSharedRef<const FOnlineLobbyId>> &Lobbies) {
                        Context->Assert().TestTrue(TEXT("Expected lobby search to succeed."), Error.bSucceeded);
                        if (!Context->Assert().TestEqual(TEXT("Expected to get only 1 lobby."), Lobbies.Num(), 1))
                        {
                            TaskOnDone.ExecuteIfBound(false);
                            return;
                        }

                        auto JoinInfoJson = Context->Instance(1).GetPartyInterface()->MakeJoinInfoJson(
                            Context->User(1),
                            Lobbies[0].Get());
                        if (!Context->Assert().TestFalse(TEXT("Join info JSON is not empty"), JoinInfoJson.IsEmpty()))
                        {
                            TaskOnDone.ExecuteIfBound(false);
                            return;
                        }

                        auto JoinInfo = Context->Instance(1).GetPartyInterface()->MakeJoinInfoFromJson(JoinInfoJson);
                        if (!Context->Assert().TestTrue(TEXT("Join info is valid"), JoinInfo.IsValid()))
                        {
                            TaskOnDone.ExecuteIfBound(false);
                            return;
                        }

                        *StoredInvite = JoinInfo;
                        TaskOnDone.ExecuteIfBound(true);
                    })));
    });

    // Join the party using the invite.
    Executor->Then<FJoinPartyViaInviteChainedTask>(1, StoredInvite);

    // Check that everyone is in the party.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()