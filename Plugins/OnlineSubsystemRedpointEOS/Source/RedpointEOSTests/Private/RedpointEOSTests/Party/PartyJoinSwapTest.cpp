// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"
#include "RedpointEOSTests/Party/RejectAllPendingInvitesChainedTask.h"
#include "RedpointEOSTests/Party/SendAndReceiveInvitationChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1855330113, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_JoinSwap,
    "Redpoint.EOS.Party.JoinSwap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_JoinSwap::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId1 = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId2 = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();

    // Create a party on instance 1.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId1);

    // Create a party on instance 2.
    Executor->Then<FCreatePartyChainedTask>(1, StoredPartyId2);

    // Ensure instance 3 is not in any parties.
    Executor->Then<FVerifyPartyMembershipChainedTask>(2, FVerifyPartyMembershipChainedTask::FInitList{});

    for (int i = 0; i < 2; i++)
    {
        // Verify that the instances are in their own parties.
        Executor->Then<FVerifyPartyMembershipChainedTask>(
            0,
            FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1});
        Executor->Then<FVerifyPartyMembershipChainedTask>(
            1,
            FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId2});

        // Send an invite from instance 1 to instance 3.
        Executor->Then<FRejectAllPendingInvitesChainedTask>(0);
        Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 2, StoredPartyId1, StoredInvite);

        // Join the party on instance 3 using the invite.
        Executor->Then<FJoinPartyViaInviteChainedTask>(2, StoredInvite);

        // Verify instance 3 is in the correct party.
        Executor->Then<FVerifyPartyMembershipChainedTask>(
            2,
            FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1});

        // Send an invite from instance 2 to instance 3.
        Executor->Then<FRejectAllPendingInvitesChainedTask>(1);
        Executor->Then<FSendAndReceiveInvitationChainedTask>(1, 2, StoredPartyId2, StoredInvite);

        // Join the party on instance 3 using the invite.
        Executor->Then<FJoinPartyViaInviteChainedTask>(2, StoredInvite);

        // Verify instance 3 is in the correct party.
        Executor->Then<FVerifyPartyMembershipChainedTask>(
            2,
            FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId2});
    }

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()