// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSOrchestrator/OrchestratorPortMapping.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4225765487, Redpoint::EOS::Orchestrator)
{

FOrchestratorPortMapping::FOrchestratorPortMapping(const FName &InUnrealEnginePort, const int16 &InExternalPort)
    : UnrealEnginePort(InUnrealEnginePort)
    , ExternalPort(InExternalPort)
{
}

FOrchestratorPortMapping::FOrchestratorPortMapping(const FOrchestratorPortMapping &InOther)
    : UnrealEnginePort(InOther.UnrealEnginePort)
    , ExternalPort(InOther.ExternalPort)
{
}

FOrchestratorPortMapping::FOrchestratorPortMapping(FOrchestratorPortMapping &&InOther)
    : UnrealEnginePort(InOther.UnrealEnginePort)
    , ExternalPort(InOther.ExternalPort)
{
}

const FName &FOrchestratorPortMapping::GetPortName() const
{
    return this->UnrealEnginePort;
}

const int16 &FOrchestratorPortMapping::GetPortValue() const
{
    return this->ExternalPort;
}

}

REDPOINT_EOS_CODE_GUARD_END()