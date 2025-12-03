// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/SendAndReceiveInvitationChainedTask.h"

#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1587078365, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;

FSendAndReceiveInvitationChainedTask::FSendAndReceiveInvitationChainedTask(
    int32 InFromInstanceId,
    int32 InToInstanceId,
    TSharedRef<TSharedPtr<const FOnlinePartyId>> InPartyId,
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> InStoredPartyJoinInfo)
    : FromInstanceId(InFromInstanceId)
    , ToInstanceId(InToInstanceId)
    , PartyId(InPartyId)
    , StoredPartyJoinInfo(InStoredPartyJoinInfo)
    , OnPartyInviteReceivedHandle()
    , TimeoutHandle()
    , bTimedOut(false)
{
}

void FSendAndReceiveInvitationChainedTask::ExecuteAsync(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    // Register an event on the target instance to see when the invite arrives.
    this->OnPartyInviteReceivedHandle =
        Context->Instance(this->ToInstanceId)
            .GetPartyInterface()
            ->AddOnPartyInviteReceivedExDelegate_Handle(FOnPartyInviteReceivedExDelegate::CreateSP(
                this,
                &FSendAndReceiveInvitationChainedTask::OnPartyInviteReceived,
                Context,
                OnDone));

    // Send the invitation and make sure it succeeds.
    Context->Instance(this->FromInstanceId)
        .GetPartyInterface()
        ->SendInvitation(
            Context->User(this->FromInstanceId),
            **this->PartyId,
            FPartyInvitationRecipient(Context->User(this->ToInstanceId)),
            FOnSendPartyInvitationComplete::CreateLambda([Context](
                                                             const FUniqueNetId &,
                                                             const FOnlinePartyId &,
                                                             const FUniqueNetId &,
                                                             const ESendPartyInvitationCompletionResult Result) {
                Context->Assert().TestEqual(
                    TEXT("Expected SendInvitation operation to succeed."),
                    Result,
                    ESendPartyInvitationCompletionResult::Succeeded);
            }));

    // Start a timer in case the invitation is never received.
    this->TimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FSendAndReceiveInvitationChainedTask::OnPartyInviteTimeout, Context, OnDone),
        20.0f);
}

void FSendAndReceiveInvitationChainedTask::OnPartyInviteReceived(
    const FUniqueNetId &LocalUserId,
    const IOnlinePartyJoinInfo &Invitation,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    // Don't handle anything if the timeout handler has already run.
    if (this->bTimedOut)
    {
        return;
    }

    // Make sure this event is for the party and user we care about.
    if (*Invitation.GetPartyId() != **this->PartyId || LocalUserId != Context->User(this->ToInstanceId))
    {
        return;
    }

    // Cancel the timeout handler.
    FTSTicker::GetCoreTicker().RemoveTicker(this->TimeoutHandle);

    // Remove the event handler.
    Context->Instance(this->ToInstanceId)
        .GetPartyInterface()
        ->ClearOnPartyInviteReceivedExDelegate_Handle(this->OnPartyInviteReceivedHandle);

    // Assert that the invitation has the expected state.
    Context->Assert().TestTrue(
        TEXT("Expected invitation to be sent by the sending user."),
        *Invitation.GetSourceUserId() == Context->User(this->FromInstanceId));

    // Assert that the receiver sees the invite in the pending invites list.
    TArray<IOnlinePartyJoinInfoConstRef> Invites;
    Context->Instance(this->ToInstanceId)
        .GetPartyInterface()
        ->GetPendingInvites(Context->User(this->ToInstanceId), Invites);
    if (Context->Assert().TestTrue(
            TEXT("Expected GetPendingInvites to have at least one invite during OnPartyInviteReceived."),
            Invites.Num() > 0))
    {
        bool bFound = false;
        for (const auto &Invite : Invites)
        {
            if (*Invite->GetPartyId() == *Invitation.GetPartyId())
            {
                bFound = true;
                break;
            }
        }
        Context->Assert().TestTrue(
            TEXT("Expected GetPendingInvites to contain invitation passed to OnPartyInviteReceived."),
            bFound);
    }

    // Return success.
    *this->StoredPartyJoinInfo = Invitation.AsShared();
    OnDone.ExecuteIfBound(true);
}

bool FSendAndReceiveInvitationChainedTask::OnPartyInviteTimeout(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    // Ensure that OnPartyInviteReceived doesn't handle anything after this.
    this->bTimedOut = true;

    // Remove the event handler.
    Context->Instance(this->ToInstanceId)
        .GetPartyInterface()
        ->ClearOnPartyInviteReceivedExDelegate_Handle(this->OnPartyInviteReceivedHandle);

    // Assert and finish the task.
    Context->Assert().TestTrue(
        TEXT("Encountered timeout while waiting for OnPartyInviteReceivedEx after calling SendInvitation!"),
        false);
    OnDone.ExecuteIfBound(false);

    // Don't tick again.
    return false;
}

}

REDPOINT_EOS_CODE_GUARD_END()