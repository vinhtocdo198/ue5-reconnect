// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/RejectAllPendingInvitesChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2147979682, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;

FRejectAllPendingInvitesChainedTask::FRejectAllPendingInvitesChainedTask(int32 InSearchingInstanceId)
    : InstanceId(InSearchingInstanceId)
    , TimeoutHandle()
    , bTimedOut(false)
{
}

void FRejectAllPendingInvitesChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    this->StartRejection(Context, OnDone);
}

void FRejectAllPendingInvitesChainedTask::StartRejection(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    TArray<IOnlinePartyJoinInfoConstRef> Invites;
    if (!Context->Assert().TestTrue(
            TEXT("Expected to be able to get pending invites for FRejectAllPendingInvitesChainedTask."),
            Context->Instance(this->InstanceId)
                .GetPartyInterface()
                ->GetPendingInvites(Context->User(this->InstanceId), Invites)))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    if (Invites.Num() == 0)
    {
        Context->Assert().AddInfo(TEXT("All pending invites have been rejected."));
        OnDone.ExecuteIfBound(true);
        return;
    }

    const auto &PendingInvite = Invites[0];

    // Register the event handler.
    Context->Instance(this->InstanceId)
        .GetPartyInterface()
        ->AddOnPartyInviteRemovedExDelegate_Handle(FOnPartyInviteRemovedExDelegate::CreateSP(
            this,
            &FRejectAllPendingInvitesChainedTask::OnPartyInviteRemoved,
            Context,
            OnDone));

    // Start our timeout operation (in case rejection fails).
    this->TimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FRejectAllPendingInvitesChainedTask::OnPartyInviteTimeout, Context, OnDone),
        10.0f);

    // Ask for the invite to be rejected. Note that because the IOnlinePartySystem::RejectInvitation function signature
    // is bad, this might not reject the invitation that we've picked if the same sending user sent two party
    // invitations to our local user. Therefore, we don't compare the invite ID against anything; we just keep doing
    // this operation until there are no invite left or a timeout occurs.
    Context->Instance(this->InstanceId)
        .GetPartyInterface()
        ->RejectInvitation(Context->User(this->InstanceId), *PendingInvite->GetSourceUserId());
}

bool FRejectAllPendingInvitesChainedTask::OnPartyInviteTimeout(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    // Ensure that OnPartyInviteRemoved doesn't handle anything after this.
    this->bTimedOut = true;

    // Assert and finish the task.
    Context->Assert().TestTrue(
        TEXT("Encountered timeout while waiting for OnPartyInviteRemovedEx after calling RejectInvitation!"),
        false);
    OnDone.ExecuteIfBound(false);

    // Don't tick again.
    return false;
}

void FRejectAllPendingInvitesChainedTask::OnPartyInviteRemoved(
    const FUniqueNetId &LocalUserId,
    const IOnlinePartyJoinInfo &Invitation,
    EPartyInvitationRemovedReason Reason,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    // Don't handle anything if the timeout handler has already run.
    if (this->bTimedOut)
    {
        return;
    }

    // Log which invitation was removed.
    Context->Assert().AddInfo(FString::Printf(
        TEXT("Pending invitation for party '%s' has been removed."),
        *Invitation.GetPartyId()->ToString()));

    // Cancel the timeout handler.
    FTSTicker::GetCoreTicker().RemoveTicker(this->TimeoutHandle);

    // Start rejecting the next invitation (this will finish the task if there are no more invitations left).
    this->StartRejection(Context, OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()