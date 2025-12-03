// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/Networking/CloseClientPIEBeaconChainedTask.h"

#include "Containers/Ticker.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(550696605, Redpoint::EOS::Editor::Tests::Networking)
{

FCloseClientPIEBeaconChainedTask::FCloseClientPIEBeaconChainedTask(
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> InBeaconClient)
    : BeaconClient(InBeaconClient)
{
}

void FCloseClientPIEBeaconChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
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

    (*this->BeaconClient)->DestroyBeacon();

    // @note: We need to wait some time to make sure the beacon shuts down and the networking state settles for other
    // checks.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSPLambda(this, [this, OnDone](float) -> bool {
        if (this->BeaconClient->IsValid())
        {
            // Still waiting for it to shutdown.
            return true;
        }

        // Shutdown is complete.
        OnDone.ExecuteIfBound(true);
        return false;
    }));
}

}

REDPOINT_EOS_CODE_GUARD_END()