// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatformRegistry.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3273657706, Redpoint::EOS::Core)
{
using namespace ::Redpoint::EOS::Platform;

#if WITH_EDITOR
typedef TDelegate<void(int32_t, const FString &, const FString &)> FOnLogForwardedForEditor;
#endif

class REDPOINTEOSCORE_API FModule : public FDefaultModuleImpl
{
private:
    TSharedRef<FRuntimePlatformRegistry> RuntimePlatformRegistry;

public:
    FModule();
    UE_NONCOPYABLE(FModule);
    virtual ~FModule() override = default;

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FORCEINLINE FModule &GetModuleChecked()
    {
        FModuleManager &ModuleManager = FModuleManager::Get();
        return ModuleManager.LoadModuleChecked<FModule>("RedpointEOSCore");
    }

    enum class EModuleLoadBehaviour : uint8
    {
        Default,

        ShuttingDown,
    };

    static FORCEINLINE FModule *GetModule(EModuleLoadBehaviour LoadBehaviour)
    {
        if (LoadBehaviour == EModuleLoadBehaviour::Default)
        {
            return &GetModuleChecked();
        }
        else
        {
            FModuleManager &ModuleManager = FModuleManager::Get();
            return (FModule *)ModuleManager.GetModule("RedpointEOSCore");
        }
    }

    TSharedRef<Redpoint::EOS::Platform::IRuntimePlatform> GetRuntimePlatform() const;
    void SetRuntimePlatform(const TSharedRef<Redpoint::EOS::Platform::IRuntimePlatform> &InRuntimePlatform);
    TSharedRef<Redpoint::EOS::Platform::FRuntimePlatformRegistry> GetRuntimePlatformRegistry() const;

#if WITH_EDITOR
    void SetLogHandler(const FOnLogForwardedForEditor &InEditorLogForwardingHandler);
#endif
};

}

namespace Redpoint::EOS::Core
{
#if WITH_EDITOR
REDPOINT_EOS_FILE_NS_EXPORT(3273657706, Redpoint::EOS::Core, FOnLogForwardedForEditor)
#endif
REDPOINT_EOS_FILE_NS_EXPORT(3273657706, Redpoint::EOS::Core, FModule)
}

REDPOINT_EOS_CODE_GUARD_END()