// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"
#include "RedpointEOSTests/Party/LeavePartyChainedTask.h"
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

namespace REDPOINT_EOS_FILE_NS_ID(4014261865, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_LeaveMultiInstance,
    "Redpoint.EOS.Party.LeaveMultiInstance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_LeaveMultiInstance::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();

    // Create a party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId);

    // Clear out any pending invitations.
    Executor->Then<FRejectAllPendingInvitesChainedTask>(0);
    Executor->Then<FRejectAllPendingInvitesChainedTask>(1);

    // Send invites from the first user to the other users.
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId, StoredInvite);

    // Join the party using the invite.
    Executor->Then<FJoinPartyViaInviteChainedTask>(1, StoredInvite);

    // Check that everyone is in the party.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});

    // Leave the party on the client.
    Executor->Then<FLeavePartyChainedTask>(1, StoredPartyId, FLeavePartyChainedTask::FObserverList{0});

    // Check that the client isn't in the party any more.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{});

    // Clear out any pending invitations.
    Executor->Then<FRejectAllPendingInvitesChainedTask>(1);

    // Send a new invitation.
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId, StoredInvite);

    // Join the party again using the invite.
    Executor->Then<FJoinPartyViaInviteChainedTask>(1, StoredInvite);

    // Check that the party state is what we expect again.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});

    // Leave the party on both the host and client.
    Executor->Then<FLeavePartyChainedTask>(1, StoredPartyId, FLeavePartyChainedTask::FObserverList{0});
    Executor->Then<FLeavePartyChainedTask>(0, StoredPartyId);

    // Check that the party state is what we expect again.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{});

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()