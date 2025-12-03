// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSOrchestrator/NullOrchestrator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1620967636, Redpoint::EOS::Orchestrator)
{

bool FNullOrchestrator::IsEnabled() const
{
    // Always allowed to be selected.
    return true;
}

void FNullOrchestrator::Init()
{
}

TTask<TArray<FOrchestratorPortMapping>> FNullOrchestrator::GetPortMappings()
{
    return TTask<TArray<FOrchestratorPortMapping>>::FromResult(TArray<FOrchestratorPortMapping>());
}

TTask<void> FNullOrchestrator::ServingTraffic(const FName &SessionName, const FString &SessionId)
{
    return TTask<void>::FromResult();
}

TTask<void> FNullOrchestrator::SessionStarted()
{
    return TTask<void>::FromResult();
}

bool FNullOrchestrator::ShouldDestroySessionOnEndSession()
{
    return false;
}

TTask<void> FNullOrchestrator::Shutdown()
{
    return TTask<void>::FromResult();
}

}

REDPOINT_EOS_CODE_GUARD_END()