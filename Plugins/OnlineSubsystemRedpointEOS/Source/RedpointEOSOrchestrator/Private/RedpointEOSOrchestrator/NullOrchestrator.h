// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSOrchestrator/Orchestrator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1620967636, Redpoint::EOS::Orchestrator)
{
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Orchestrator;

class REDPOINTEOSORCHESTRATOR_API FNullOrchestrator : public IOrchestrator
{
public:
    FNullOrchestrator() = default;
    UE_NONCOPYABLE(FNullOrchestrator);
    virtual ~FNullOrchestrator() override = default;

    [[nodiscard]] virtual bool IsEnabled() const override;

    virtual void Init() override;
    [[nodiscard]] virtual TTask<TArray<FOrchestratorPortMapping>> GetPortMappings() override;
    [[nodiscard]] virtual TTask<void> ServingTraffic(const FName &SessionName, const FString &SessionId) override;
    [[nodiscard]] virtual TTask<void> SessionStarted() override;
    [[nodiscard]] virtual bool ShouldDestroySessionOnEndSession() override;
    [[nodiscard]] virtual TTask<void> Shutdown() override;
};

}

namespace Redpoint::EOS::Orchestrator
{
REDPOINT_EOS_FILE_NS_EXPORT(1620967636, Redpoint::EOS::Orchestrator, FNullOrchestrator)
}

REDPOINT_EOS_CODE_GUARD_END()