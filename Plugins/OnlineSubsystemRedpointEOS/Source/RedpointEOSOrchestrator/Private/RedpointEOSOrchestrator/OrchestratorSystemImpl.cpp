// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSOrchestrator/OrchestratorSystemImpl.h"

#include "RedpointEOSOrchestrator/AgonesOrchestrator.h"
#include "RedpointEOSOrchestrator/Logging.h"
#include "RedpointEOSOrchestrator/NullOrchestrator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(143096825, Redpoint::EOS::Orchestrator)
{
using namespace ::Redpoint::EOS::Orchestrator;

TSharedRef<IOrchestrator> FOrchestratorSystemImpl::SelectOrchestrator(const FPlatformHandle &PlatformHandle)
{
    if (PlatformHandle->GetEnvironment()->bIsDedicatedServer)
    {
        UE_LOG(LogRedpointEOSOrchestrator, Verbose, TEXT("Determining server orchestrator..."));

        FString IgnoreOverridePort = FPlatformMisc::GetEnvironmentVariable(TEXT("EOS_IGNORE_ORCHESTRATOR"));
        if (IgnoreOverridePort != TEXT("true"))
        {
#if REDPOINT_EOS_ORCHESTRATORS_ENABLED
            // Check if we're running on Agones.
            {
                auto Agones = MakeShared<FAgonesOrchestrator>();
                if (Agones->IsEnabled())
                {
                    UE_LOG(LogRedpointEOSOrchestrator, Verbose, TEXT("Selected Agones as the server orchestrator."));
                    return Agones;
                }
            }
#endif
        }
        else
        {
            UE_LOG(
                LogRedpointEOSOrchestrator,
                Verbose,
                TEXT("Null server orchestrator will be selected because the environment variable "
                     "EOS_IGNORE_ORCHESTRATOR is set to 'true'."));
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSOrchestrator,
            Verbose,
            TEXT("Null server orchestrator will be selected because this platform instance is not running as a "
                 "dedicated server."));
    }

    // If we have no other implementations, use the null orchestrator.
    UE_LOG(LogRedpointEOSOrchestrator, Verbose, TEXT("Selected null implementation as the server orchestrator."));
    return MakeShared<FNullOrchestrator>();
}

FOrchestratorSystemImpl::FOrchestratorSystemImpl(const FPlatformHandle &PlatformHandle)
    : SelectedOrchestrator(SelectOrchestrator(PlatformHandle))
{
    this->SelectedOrchestrator->Init();
}

TTask<TArray<FOrchestratorPortMapping>> FOrchestratorSystemImpl::GetPortMappings()
{
    return this->SelectedOrchestrator->GetPortMappings();
}

TTask<void> FOrchestratorSystemImpl::SessionCreated(const FName &SessionName, const FString &SessionId)
{
    return this->SelectedOrchestrator->ServingTraffic(SessionName, SessionId);
}

TTask<void> FOrchestratorSystemImpl::SessionStarted()
{
    return this->SelectedOrchestrator->SessionStarted();
}

bool FOrchestratorSystemImpl::ShouldDestroySessionOnEndSession()
{
    return this->SelectedOrchestrator->ShouldDestroySessionOnEndSession();
}

TTask<void> FOrchestratorSystemImpl::SessionEnded()
{
    if (this->SelectedOrchestrator->ShouldDestroySessionOnEndSession())
    {
        return TTask<void>::FromResult();
    }
    else
    {
        return this->SelectedOrchestrator->Shutdown();
    }
}

TTask<void> FOrchestratorSystemImpl::SessionDestroyed()
{
    return this->SelectedOrchestrator->Shutdown();
}

}

REDPOINT_EOS_CODE_GUARD_END()