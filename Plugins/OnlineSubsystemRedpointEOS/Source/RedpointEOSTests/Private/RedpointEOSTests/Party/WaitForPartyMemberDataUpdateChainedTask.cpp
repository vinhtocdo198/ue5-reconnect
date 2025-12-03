// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/WaitForPartyMemberDataUpdateChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3390687173, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

FWaitForPartyMemberDataUpdateChainedTask::FWaitForPartyMemberDataUpdateChainedTask(
    TSharedRef<FPartyMemberDataUpdateStateHandle> InUpdateState)
    : UpdateState(InUpdateState)
{
}

void FWaitForPartyMemberDataUpdateChainedTask::ExecuteAsync(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    if (this->UpdateState->bHandled)
    {
        OnDone.ExecuteIfBound(true);
    }
    else
    {
        this->UpdateState->OnTimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(this, &FWaitForPartyMemberDataUpdateChainedTask::OnTimeout, Context, OnDone),
            20.f);
        this->UpdateState->OnReceivedFunction = OnDone;
    }
}

bool FWaitForPartyMemberDataUpdateChainedTask::OnTimeout(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (this->UpdateState->bHandled)
    {
        return false;
    }

    Context->Assert().TestTrue(TEXT("Got timeout while waiting for party member data."), false);
    this->UpdateState->PartySystem.Pin()->ClearOnPartyDataReceivedDelegate_Handle(
        this->UpdateState->OnPartyDataReceivedHandle);
    OnDone.ExecuteIfBound(false);
    return false;
}

}

REDPOINT_EOS_CODE_GUARD_END()