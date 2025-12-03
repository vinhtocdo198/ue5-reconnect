// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/Networking/WaitForClientBeaconToDestroyItselfAfterPingChainedTask.h"

#include "Containers/Ticker.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(468860297, Redpoint::EOS::Editor::Tests::Networking)
{

FWaitForClientBeaconToDestroyItselfAfterPingChainedTask::FWaitForClientBeaconToDestroyItselfAfterPingChainedTask(
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> InBeaconClient)
    : BeaconClient(InBeaconClient)
{
}

void FWaitForClientBeaconToDestroyItselfAfterPingChainedTask::ExecuteAsync(
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

    // Wait for the beacons to shutdown.
    auto StartTime = FDateTime::UtcNow();
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSPLambda(this, [this, OnDone, Context, StartTime](float) -> bool {
            if (!this->BeaconClient->IsValid())
            {
                OnDone.ExecuteIfBound(true);
                return false;
            }
            if ((*this->BeaconClient)->GetConnectionState() == EBeaconConnectionState::Closed ||
                (*this->BeaconClient)->GetConnectionState() == EBeaconConnectionState::Invalid)
            {
                (*this->BeaconClient)->Destroy();
                OnDone.ExecuteIfBound(true);
                return false;
            }
            if ((FDateTime::UtcNow() - StartTime).GetTotalSeconds() > 60.0)
            {
                Context->Assert().TestTrue(TEXT("Expected beacons to automatically close after ping."), false);
                OnDone.ExecuteIfBound(false);
                return false;
            }
            return true;
        }));
}

}

REDPOINT_EOS_CODE_GUARD_END()