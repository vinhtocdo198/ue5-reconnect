// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_ORCHESTRATORS_ENABLED
#include "Containers/Ticker.h"
#include "HttpModule.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSOrchestrator/Orchestrator.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_ORCHESTRATORS_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1636037717, Redpoint::EOS::Orchestrator)
{
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Orchestrator;

class REDPOINTEOSORCHESTRATOR_API FAgonesOrchestrator : public IOrchestrator,
                                                        public TSharedFromThis<FAgonesOrchestrator>
{
private:
    int16 AgonesHttpServerPort;
    FTSTicker::FDelegateHandle HealthCheckTimerHandle;
    bool bHasSentReady;
    bool bHasSentAllocate;
    bool bHasSentShutdown;

    typedef TDelegate<void(const TArray<FOrchestratorPortMapping> &Mappings)> FOnGetPortMappingsComplete;

public:
    FAgonesOrchestrator();
    UE_NONCOPYABLE(FAgonesOrchestrator);
    virtual ~FAgonesOrchestrator() override;

private:
    void HandleHealthCheck(FHttpRequestPtr HttpRequest, const FHttpResponsePtr HttpResponse, const bool bSucceeded);
    bool SendHealthCheck(float DeltaSeconds);

    bool RetryGetPortMappings(float DeltaSeconds, FOnGetPortMappingsComplete OnComplete);
    void HandleGameServer(
        FHttpRequestPtr HttpRequest,
        const FHttpResponsePtr HttpResponse,
        const bool bSucceeded,
        FOnGetPortMappingsComplete OnComplete);
    void HandleLabel(
        FHttpRequestPtr HttpRequest,
        const FHttpResponsePtr HttpResponse,
        const bool bSucceeded,
        FSimpleDelegate OnComplete);
    void HandleReady(
        FHttpRequestPtr HttpRequest,
        const FHttpResponsePtr HttpResponse,
        const bool bSucceeded,
        FSimpleDelegate OnComplete);
    void HandleAllocate(
        FHttpRequestPtr HttpRequest,
        const FHttpResponsePtr HttpResponse,
        const bool bSucceeded,
        FSimpleDelegate OnComplete);
    void HandleShutdown(
        FHttpRequestPtr HttpRequest,
        const FHttpResponsePtr HttpResponse,
        const bool bSucceeded,
        FSimpleDelegate OnComplete);

public:
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
REDPOINT_EOS_FILE_NS_EXPORT(1636037717, Redpoint::EOS::Orchestrator, FAgonesOrchestrator)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()