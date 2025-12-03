// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/LeavePartyChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1731051311, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

FLeavePartyChainedTask::FLeavePartyChainedTask(
    int32 InInstanceId,
    TSharedRef<TSharedPtr<const FOnlinePartyId>> InPartyId,
    FObserverList InObservingInstanceIds)
    : InstanceId(InInstanceId)
    , PartyId(InPartyId)
    , ObservingInstanceIds()
    , ObservingInstanceExitedDelegateHandles()
    , ObservingInstanceSeenExit()
    , bLeavePartySucceeded()
    , bFiredOnDone(false)
{
    for (const auto &ObservingInstanceId : InObservingInstanceIds)
    {
        ObservingInstanceIds.Add(ObservingInstanceId);
        ObservingInstanceSeenExit.Add(false);
    }
}

void FLeavePartyChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    // Register events on observing instances to know when they have seen the target user leave.
    for (int i = 0; i < this->ObservingInstanceIds.Num(); i++)
    {
        ObservingInstanceExitedDelegateHandles.Add(
            Context->Instance(this->ObservingInstanceIds[i])
                .GetPartyInterface()
                ->AddOnPartyMemberExitedDelegate_Handle(FOnPartyMemberExitedDelegate::CreateSP(
                    this,
                    &FLeavePartyChainedTask::OnMemberExited,
                    i,
                    Context,
                    OnDone)));
    }

    // Execute the leave on the target instance.
    Context->Instance(this->InstanceId)
        .GetPartyInterface()
        ->LeaveParty(
            Context->User(this->InstanceId),
            **this->PartyId,
            true,
            FOnLeavePartyComplete::CreateSP(this, &FLeavePartyChainedTask::OnLeavePartyComplete, Context, OnDone));
}

void FLeavePartyChainedTask::OnMemberExited(
    const FUniqueNetId &,
    const FOnlinePartyId &InPartyId,
    const FUniqueNetId &InMemberId,
    const EMemberExitedReason,
    int ObserverInstanceIndex,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (PartyId == this->PartyId && InMemberId == Context->User(this->InstanceId))
    {
        this->ObservingInstanceSeenExit[ObserverInstanceIndex] = true;
        this->CheckReadyForReturn(Context, OnDone);
    }
}

void FLeavePartyChainedTask::OnLeavePartyComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &LeavePartyId,
    const ELeavePartyCompletionResult Result,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    bool bSucceeded = Context->Assert().TestEqual(
        FString::Printf(TEXT("Expected ELeavePartyCompletionResult to be successful (got %d instead)."), (int32)Result),
        Result,
        ELeavePartyCompletionResult::Succeeded);

    if (bSucceeded)
    {
        TArray<TSharedRef<const FOnlinePartyId>> PartyIds;
        Context->Assert().TestTrue(
            TEXT("Expected GetJoinedParties to succeed."),
            Context->Instance(this->InstanceId)
                .GetPartyInterface()
                ->GetJoinedParties(Context->User(this->InstanceId), PartyIds));
        for (const auto &CheckedPartyId : PartyIds)
        {
            if (!Context->Assert().TestNotEqual(
                    TEXT("Expected left party to no longer be returned by GetJoinedParties after LeaveParty succeeds!"),
                    *CheckedPartyId,
                    LeavePartyId))
            {
                bSucceeded = false;
            }
        }
    }

    this->bLeavePartySucceeded = bSucceeded;

    this->CheckReadyForReturn(Context, OnDone);
}

void FLeavePartyChainedTask::CheckReadyForReturn(FChainedTaskContextRef Context, FOnComplete OnDone)
{
    bool bReady = true;
    if (!this->bLeavePartySucceeded.IsSet())
    {
        // We haven't finished the LeaveParty async call yet.
        bReady = false;
    }
    else if (!this->bLeavePartySucceeded.GetValue())
    {
        // bReady = true, because the leave failed and we won't see events.
    }
    else
    {
        for (int i = 0; i < this->ObservingInstanceSeenExit.Num(); i++)
        {
            if (!this->ObservingInstanceSeenExit[i])
            {
                // This instance hasn't seen the leave yet.
                bReady = false;
                break;
            }
        }
    }

    if (!bReady)
    {
        return;
    }

    if (bReady)
    {
        if (!bFiredOnDone)
        {
            for (int i = 0; i < this->ObservingInstanceIds.Num(); i++)
            {
                Context->Instance(this->ObservingInstanceIds[i])
                    .GetPartyInterface()
                    ->ClearOnPartyMemberExitedDelegate_Handle(this->ObservingInstanceExitedDelegateHandles[i]);
            }

            bFiredOnDone = true;
            OnDone.ExecuteIfBound(this->bLeavePartySucceeded.GetValue());
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()