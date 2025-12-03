// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/Networking/WaitForClientPIEBeaconPongChainedTask.h"

#include "Containers/Ticker.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(458620805, Redpoint::EOS::Editor::Tests::Networking)
{

FWaitForClientPIEBeaconPongChainedTask::FWaitForClientPIEBeaconPongChainedTask(
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> InBeaconClient)
    : BeaconClient(InBeaconClient)
{
}

void FWaitForClientPIEBeaconPongChainedTask::ExecuteAsync(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    if (!Context->Assert().TestTrue(TEXT("Expected beacon client to be set"), this->BeaconClient->IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    if (!Context->Assert().TestEqual(
            TEXT("Expected beacon state to be Open"),
            (*this->BeaconClient)->GetConnectionState(),
            EBeaconConnectionState::Open))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSPLambda(this, [this, OnDone](float) -> bool {
        if (!(*this->BeaconClient)->bClientGotPong)
        {
            // Still waiting for it to get a server ping.
            return true;
        }

        // @todo: Timeout.

        // Received the ping.
        OnDone.ExecuteIfBound(true);
        return false;
    }));
}

}

REDPOINT_EOS_CODE_GUARD_END()