// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"
#include "RedpointEOSTests/Party/RejectAllPendingInvitesChainedTask.h"
#include "RedpointEOSTests/Party/SendAndReceiveInvitationChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/DelayChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1934901567, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_Kicked,
    "Redpoint.EOS.Party.Kicked",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_Kicked::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();
    TSharedRef<FDelegateHandle> StoredPartyMemberExitedHost = MakeShared<FDelegateHandle>();
    TSharedRef<FDelegateHandle> StoredPartyMemberExitedClient = MakeShared<FDelegateHandle>();
    TSharedRef<FDelegateHandle> StoredPartyExitedClient = MakeShared<FDelegateHandle>();
    TSharedRef<bool> bSawMemberKickOnHost = MakeShared<bool>(false);
    TSharedRef<bool> bSawMemberKickOnClient = MakeShared<bool>(false);
    TSharedRef<bool> bSawExitedOnClient = MakeShared<bool>(false);

    // Create a party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId);

    // Clear out any pending invitations so that we will only see the one we're about to send.
    Executor->Then<FRejectAllPendingInvitesChainedTask>(1);

    // Send invites from the first user to the other user.
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId, StoredInvite);

    // Join the party using the invite.
    Executor->Then<FJoinPartyViaInviteChainedTask>(1, StoredInvite);
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});

    // Register event handlers.
    Executor->Then<FLambdaChainedTask>([StoredPartyId,
                                        StoredPartyMemberExitedHost,
                                        StoredPartyMemberExitedClient,
                                        StoredPartyExitedClient,
                                        bSawMemberKickOnHost,
                                        bSawMemberKickOnClient,
                                        bSawExitedOnClient](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        *StoredPartyMemberExitedHost = Context->Instance(0).GetPartyInterface()->AddOnPartyMemberExitedDelegate_Handle(
            FOnPartyMemberExitedDelegate::CreateLambda([StoredPartyId, bSawMemberKickOnHost, Context](
                                                           const FUniqueNetId &LocalUserId,
                                                           const FOnlinePartyId &PartyId,
                                                           const FUniqueNetId &MemberId,
                                                           const EMemberExitedReason Reason) {
                if (LocalUserId == Context->User(0) && PartyId == **StoredPartyId && MemberId == Context->User(1))
                {
                    Context->Assert().TestEqual(
                        TEXT("Expected MemberExitedReason to be Kicked on host."),
                        Reason,
                        EMemberExitedReason::Kicked);
                    *bSawMemberKickOnHost = true;
                }
            }));
        *StoredPartyMemberExitedClient =
            Context->Instance(1).GetPartyInterface()->AddOnPartyMemberExitedDelegate_Handle(
                FOnPartyMemberExitedDelegate::CreateLambda([StoredPartyId, bSawMemberKickOnClient, Context](
                                                               const FUniqueNetId &LocalUserId,
                                                               const FOnlinePartyId &PartyId,
                                                               const FUniqueNetId &MemberId,
                                                               const EMemberExitedReason Reason) {
                    if (LocalUserId == Context->User(1) && PartyId == **StoredPartyId && MemberId == Context->User(1))
                    {
                        Context->Assert().TestEqual(
                            TEXT("Expected MemberExitedReason to be Kicked on client."),
                            Reason,
                            EMemberExitedReason::Kicked);
                        *bSawMemberKickOnClient = true;
                    }
                }));
        *StoredPartyExitedClient = Context->Instance(1).GetPartyInterface()->AddOnPartyExitedDelegate_Handle(
            FOnPartyExitedDelegate::CreateLambda([StoredPartyId, bSawExitedOnClient, Context](
                                                     const FUniqueNetId &LocalUserId,
                                                     const FOnlinePartyId &PartyId) {
                if (LocalUserId == Context->User(1) && PartyId == **StoredPartyId)
                {
                    *bSawExitedOnClient = true;
                }
            }));
        TaskOnDone.ExecuteIfBound(true);
    });

    // Kick the member.
    Executor->Then<FLambdaChainedTask>(
        [StoredPartyId](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            Context->Assert().TestTrue(
                TEXT("Expected KickMember to return true."),
                Context->Instance(0).GetPartyInterface()->KickMember(
                    Context->User(0),
                    **StoredPartyId,
                    Context->User(1),
                    FOnKickPartyMemberComplete::CreateLambda([Context, TaskOnDone, StoredPartyId](
                                                                 const FUniqueNetId &LocalUserId,
                                                                 const FOnlinePartyId &PartyId,
                                                                 const FUniqueNetId &MemberId,
                                                                 const EKickMemberCompletionResult Result) {
                        Context->Assert().TestTrue(
                            TEXT("Expected KickMember callback LocalUserId to be local user."),
                            LocalUserId == Context->User(0));
                        Context->Assert().TestTrue(
                            TEXT("Expected KickMember callback PartyId to be stored party ID."),
                            PartyId == **StoredPartyId);
                        Context->Assert().TestTrue(
                            TEXT("Expected KickMember callback MemberId to be target user."),
                            MemberId == Context->User(1));
                        Context->Assert().TestEqual(
                            TEXT("Expected KickMember callback Result to be successful."),
                            Result,
                            EKickMemberCompletionResult::Succeeded);
                        TaskOnDone.ExecuteIfBound(true);
                    })));
        });

    // @todo: Wait until condition instead of hard delay.
    Executor->Then<FDelayChainedTask>(5.0f);
    Executor->Then<FLambdaChainedTask>([bSawMemberKickOnHost, bSawMemberKickOnClient, bSawExitedOnClient](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        Context->Assert().TestTrue(TEXT("Expected to see party member kicked on host."), *bSawMemberKickOnHost);
        Context->Assert().TestTrue(TEXT("Expected to see party member kicked on client."), *bSawMemberKickOnClient);
        Context->Assert().TestTrue(TEXT("Expected to see party exited on client."), *bSawExitedOnClient);
        TaskOnDone.ExecuteIfBound(true);
    });
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{StoredPartyId});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{});

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()