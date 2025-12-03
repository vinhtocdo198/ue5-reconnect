// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSOrchestrator/OrchestratorPortMapping.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1278504577, Redpoint::EOS::Orchestrator)
{
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Orchestrator;

class REDPOINTEOSORCHESTRATOR_API IOrchestrator
{
public:
    IOrchestrator() = default;
    UE_NONCOPYABLE(IOrchestrator);
    virtual ~IOrchestrator() = default;

    /**
     * If this returns true, the dedicated server is running under this orchestrator and should use this implementation.
     */
    [[nodiscard]] virtual bool IsEnabled() const = 0;

    /**
     * Initializes the server orchestrator intergration. This happens at startup and is expected to run for the lifetime
     * of the dedicated server process. This is where you could, e.g. set up health check loops.
     */
    virtual void Init() = 0;

    /**
     * Fetches the port mappings for the dedicated server. If a mapping for NAME_GamePort is returned, this will be set
     * to __EOS_OverrideAddressBound. For any other ports, they are exposed as Port_<Name> in the session attribute
     * list.
     */
    [[nodiscard]] virtual TTask<TArray<FOrchestratorPortMapping>> GetPortMappings() = 0;

    /**
     * Inform the orchestrator that we are now ready to receive game traffic; this is called when CreateSession is
     * called.
     */
    [[nodiscard]] virtual TTask<void> ServingTraffic(const FName &SessionName, const FString &SessionId) = 0;

    /**
     * Inform the orchestrator that the match has started and that this game server process should not be terminated.
     * This is called by the plugin when StartSession is called.
     */
    [[nodiscard]] virtual TTask<void> SessionStarted() = 0;

    /**
     * If true, the game should treat an EndSession call as DestroySession. This is the case for orchestrators that
     * will terminate the game server process when Shutdown is called from EndSession, since in these cases you don't
     * want a stale unusable session left in the session list to expire.
     */
    [[nodiscard]] virtual bool ShouldDestroySessionOnEndSession() = 0;

    /**
     * Inform the orchestrator that the game server should be shutdown and deallocated. This is called by the plugin
     * when either EndSession or DestroySession is called.
     */
    [[nodiscard]] virtual TTask<void> Shutdown() = 0;
};

}

namespace Redpoint::EOS::Orchestrator
{
REDPOINT_EOS_FILE_NS_EXPORT(1278504577, Redpoint::EOS::Orchestrator, IOrchestrator)
}

REDPOINT_EOS_CODE_GUARD_END()