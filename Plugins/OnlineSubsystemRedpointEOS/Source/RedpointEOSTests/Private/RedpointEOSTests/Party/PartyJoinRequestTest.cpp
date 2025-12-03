// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaOverlayChainedTask.h"
#include "RedpointEOSTests/Party/LeavePartyChainedTask.h"
#include "RedpointEOSTests/Party/RejectAllPendingInvitesChainedTask.h"
#include "RedpointEOSTests/Party/SendAndReceiveInvitationChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3753122574, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_JoinRequest,
    "Redpoint.EOS.Party.JoinRequest",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_JoinRequest::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();

    // Create a party on instance 1.
    Executor->Then<FCreatePartyChainedTask>(
        0,
        StoredPartyId,
        IOnlinePartySystem::GetPrimaryPartyTypeId(),
        false,
        EJoinRequestAction::Manual);

    // Send an invite from instance 1 to instance 2.
    Executor->Then<FRejectAllPendingInvitesChainedTask>(0);
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId, StoredInvite);

    // Join the party emulating an overlay accept, followed by approval.
    Executor->Then<FJoinPartyViaOverlayChainedTask>(1, StoredInvite, false, true);

    // Verify both users are in the parties we expect.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});

    // Leave the party as instance 2.
    Executor->Then<FLeavePartyChainedTask>(1, StoredPartyId, FLeavePartyChainedTask::FObserverList{0});

    // Send an invite from instance 1 to instance 2.
    Executor->Then<FLambdaChainedTask>(
        [StoredInvite](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            StoredInvite->Reset();
            TaskOnDone.ExecuteIfBound(true);
        });
    Executor->Then<FRejectAllPendingInvitesChainedTask>(0);
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId, StoredInvite);

    // Join the party emulating an overlay accept, followed by denial.
    Executor->Then<FJoinPartyViaOverlayChainedTask>(1, StoredInvite, false, false);

    // Verify instance 2 did not join the party.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{});

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()