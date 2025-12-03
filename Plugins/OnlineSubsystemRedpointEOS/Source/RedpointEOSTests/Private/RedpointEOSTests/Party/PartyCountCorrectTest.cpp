// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"
#include "RedpointEOSTests/Party/RejectAllPendingInvitesChainedTask.h"
#include "RedpointEOSTests/Party/SendAndReceiveInvitationChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(912312239, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_CountCorrect,
    "Redpoint.EOS.Party.CountCorrect",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_CountCorrect::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{4}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite1 =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite2 =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite3 =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();

    // Create a party as the first user.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId);

    // Clear out any pending invitations so that we will only see the ones we're about to send.
    Executor->Then<FRejectAllPendingInvitesChainedTask>(0);
    Executor->Then<FRejectAllPendingInvitesChainedTask>(1);
    Executor->Then<FRejectAllPendingInvitesChainedTask>(2);
    Executor->Then<FRejectAllPendingInvitesChainedTask>(3);

    // Send invites from the first user to the other users.
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId, StoredInvite1);
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 2, StoredPartyId, StoredInvite2);
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 3, StoredPartyId, StoredInvite3);

    // Join the party using the invites.
    Executor->Then<FJoinPartyViaInviteChainedTask>(1, StoredInvite1);
    Executor->Then<FJoinPartyViaInviteChainedTask>(2, StoredInvite2);
    Executor->Then<FJoinPartyViaInviteChainedTask>(3, StoredInvite3);

    // Check that everyone is in the party.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(2, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(3, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()