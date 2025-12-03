// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSOrchestrator/OrchestratorPortMapping.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(92972731, Redpoint::EOS::Orchestrator)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Orchestrator;

class REDPOINTEOSORCHESTRATOR_API IOrchestratorSystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<IOrchestratorSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IOrchestratorSystem() = default;
    UE_NONCOPYABLE(IOrchestratorSystem);
    virtual ~IOrchestratorSystem() override = default;

    /**
     * Returns the port mappings for the dedicated game server running under the orchestrator. This is used to populate
     additional session attributes when CreateSession is called.
     *
     * In the future this will be moved to the room system hook that also calls SessionCreated.
     */
    [[nodiscard]] virtual TTask<TArray<FOrchestratorPortMapping>> GetPortMappings() = 0;

    /**
     * Called by the plugin when CreateSession has been called.
     *
     * In the future this will be moved to a room system hook.
     */
    [[nodiscard]] virtual TTask<void> SessionCreated(const FName &SessionName, const FString &SessionId) = 0;

    /**
     * Called by the plugin when StartSession has been called.
     *
     * In the future this will be moved to a room system hook.
     */
    [[nodiscard]] virtual TTask<void> SessionStarted() = 0;

    /**
     * Called by the plugin when EndSession has been called, and used to determine if the game should treat this as a
     * call to DestroySession instead.
     *
     * In the future this will be moved to a room system hook.
     */
    [[nodiscard]] virtual bool ShouldDestroySessionOnEndSession() = 0;

    /**
     * Called by the plugin when EndSession has been called.
     *
     * In the future this will be moved to a room system hook.
     */
    [[nodiscard]] virtual TTask<void> SessionEnded() = 0;

    /**
     * Called by the plugin when DestroySession has been called.
     *
     * In the future this will be moved to a room system hook.
     */
    [[nodiscard]] virtual TTask<void> SessionDestroyed() = 0;
};

}

namespace Redpoint::EOS::Orchestrator
{
REDPOINT_EOS_FILE_NS_EXPORT(92972731, Redpoint::EOS::Orchestrator, IOrchestratorSystem)
}

REDPOINT_EOS_CODE_GUARD_END()