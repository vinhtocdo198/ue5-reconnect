// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"
#include "RedpointEOSTests/Party/RejectAllPendingInvitesChainedTask.h"
#include "RedpointEOSTests/Party/SendAndReceiveInvitationChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1473543708, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_AutoLeave,
    "Redpoint.EOS.Party.AutoLeave",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_AutoLeave::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId1 = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId2 = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredPartyJoinInfo1 =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredPartyJoinInfo2 =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();

    // Create a party as the first user.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId1);

    // Create a party as the second user.
    Executor->Then<FCreatePartyChainedTask>(1, StoredPartyId2);

    // Join the first user's party.
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 2, StoredPartyId1, StoredPartyJoinInfo1);
    Executor->Then<FJoinPartyViaInviteChainedTask>(2, StoredPartyJoinInfo1);
    Executor->Then<FVerifyPartyMembershipChainedTask>(2, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId1});

    // Join the second user's party.
    Executor->Then<FSendAndReceiveInvitationChainedTask>(1, 2, StoredPartyId2, StoredPartyJoinInfo2);
    Executor->Then<FJoinPartyViaInviteChainedTask>(2, StoredPartyJoinInfo2);
    Executor->Then<FVerifyPartyMembershipChainedTask>(2, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId2});

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()