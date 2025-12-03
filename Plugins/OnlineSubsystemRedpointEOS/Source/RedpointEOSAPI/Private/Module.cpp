// Copyright June Rhodes. All Rights Reserved.

#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_GAMEPLAY_DEBUGGER_CORE

namespace Redpoint::EOS::API
{
void RegisterLiveCallGameplayDebuggerInstance();
void UnregisterLiveCallGameplayDebuggerInstance();
}

#endif

class REDPOINTEOSAPI_API FRedpointEOSAPIModule : public FDefaultModuleImpl
{
public:
    virtual void StartupModule() override
    {
        bool bSupportsAutomaticRetry = true;
        if (!GConfig->GetBool(
                TEXT("EpicOnlineServices"),
                TEXT("bEnableAutomaticRetries"),
                bSupportsAutomaticRetry,
                GEngineIni))
        {
            /* If there is no configuration value, default to true. */
            bSupportsAutomaticRetry = true;
        }
        if (!bSupportsAutomaticRetry)
        {
            UE_LOG(
                LogRedpointEOSAPI,
                Error,
                TEXT("This game has bEnableAutomaticRetries=False set in the DefaultEngine.ini and is running in an "
                     "unsupported configuration! You may experience bugs and incorrect behaviour with this option "
                     "turned off."));
        }

#if WITH_GAMEPLAY_DEBUGGER_CORE
        FCoreDelegates::OnPostEngineInit.AddLambda([]() {
            ::Redpoint::EOS::API::RegisterLiveCallGameplayDebuggerInstance();
        });
#endif
    }

#if WITH_GAMEPLAY_DEBUGGER_CORE
    virtual void ShutdownModule() override
    {
        ::Redpoint::EOS::API::UnregisterLiveCallGameplayDebuggerInstance();
    }
#endif
};

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FRedpointEOSAPIModule, RedpointEOSAPI);