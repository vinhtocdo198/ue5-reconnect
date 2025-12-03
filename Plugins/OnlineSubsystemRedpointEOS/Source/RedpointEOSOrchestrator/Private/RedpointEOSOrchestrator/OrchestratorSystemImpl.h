// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSOrchestrator/Orchestrator.h"
#include "RedpointEOSOrchestrator/OrchestratorSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(143096825, Redpoint::EOS::Orchestrator)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Orchestrator;

class FOrchestratorSystemImpl : public IOrchestratorSystem, public TSharedFromThis<FOrchestratorSystemImpl>
{
private:
    TSharedRef<IOrchestrator> SelectedOrchestrator;

    static TSharedRef<IOrchestrator> SelectOrchestrator(const FPlatformHandle &PlatformHandle);

public:
    FOrchestratorSystemImpl(const FPlatformHandle &PlatformHandle);
    UE_NONCOPYABLE(FOrchestratorSystemImpl);
    virtual ~FOrchestratorSystemImpl() override = default;

    virtual TTask<TArray<FOrchestratorPortMapping>> GetPortMappings() override;
    virtual TTask<void> SessionCreated(const FName &SessionName, const FString &SessionId) override;
    virtual TTask<void> SessionStarted() override;
    virtual bool ShouldDestroySessionOnEndSession() override;
    virtual TTask<void> SessionEnded() override;
    virtual TTask<void> SessionDestroyed() override;
};

}

namespace Redpoint::EOS::Orchestrator
{
REDPOINT_EOS_FILE_NS_EXPORT(143096825, Redpoint::EOS::Orchestrator, FOrchestratorSystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()