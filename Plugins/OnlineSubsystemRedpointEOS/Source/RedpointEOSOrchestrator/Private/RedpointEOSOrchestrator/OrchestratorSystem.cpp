// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSOrchestrator/OrchestratorSystem.h"

#include "RedpointEOSOrchestrator/OrchestratorSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(92972731, Redpoint::EOS::Orchestrator)
{
using namespace ::Redpoint::EOS::Orchestrator;

FName IOrchestratorSystem::GetSystemName()
{
    return FName(TEXT("IOrchestratorSystem"));
}

TSharedRef<IOrchestratorSystem> IOrchestratorSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    return MakeShared<FOrchestratorSystemImpl>(InPlatformHandle);
}

}

REDPOINT_EOS_CODE_GUARD_END()