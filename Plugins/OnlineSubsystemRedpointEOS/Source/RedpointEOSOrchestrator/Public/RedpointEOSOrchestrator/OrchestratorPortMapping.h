// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4225765487, Redpoint::EOS::Orchestrator)
{

struct REDPOINTEOSORCHESTRATOR_API FOrchestratorPortMapping
{
private:
    FName UnrealEnginePort;
    int16 ExternalPort;

public:
    FOrchestratorPortMapping(const FName &InUnrealEnginePort, const int16 &InExternalPort);
    FOrchestratorPortMapping(const FOrchestratorPortMapping &InOther);
    FOrchestratorPortMapping(FOrchestratorPortMapping &&InOther);
    ~FOrchestratorPortMapping() = default;

    const FName &GetPortName() const;
    const int16 &GetPortValue() const;
};

}

namespace Redpoint::EOS::Orchestrator
{
REDPOINT_EOS_FILE_NS_EXPORT(4225765487, Redpoint::EOS::Orchestrator, FOrchestratorPortMapping)
}

REDPOINT_EOS_CODE_GUARD_END()