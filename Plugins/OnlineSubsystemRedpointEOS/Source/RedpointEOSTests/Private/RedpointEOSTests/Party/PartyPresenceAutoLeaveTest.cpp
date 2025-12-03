// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1097463985, Redpoint::EOS::Tests::Party)
{

using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_PresenceAutoLeave,
    "Redpoint.EOS.Party.PresenceAutoLeave",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_PresenceAutoLeave::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId1 = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId2 = MakeShared<TSharedPtr<const FOnlinePartyId>>();

    // Create the first party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId1);

    // Create the second party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId2);

    // Verify that we're only in the second party, as the first should have been left.
    Executor->Then<FLambdaChainedTask>([StoredPartyId2](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        TArray<TSharedRef<const FOnlinePartyId>> PartyIds;
        Context->Assert().TestTrue(
            TEXT("Expected GetJoinedParties to return true."),
            Context->Instance(0).GetPartyInterface()->GetJoinedParties(Context->User(0), PartyIds));
        if (Context->Assert().TestEqual(TEXT("Expected party list to only have 1 entry."), PartyIds.Num(), 1))
        {
            Context->Assert().TestTrue(TEXT("Expected only entry to be party 2."), *PartyIds[0] == **StoredPartyId2);
        }
        TaskOnDone.ExecuteIfBound(true);
    });

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()