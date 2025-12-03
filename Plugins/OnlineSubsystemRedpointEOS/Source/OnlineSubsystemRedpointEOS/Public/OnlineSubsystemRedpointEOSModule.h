// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Modules/ModuleManager.h"
#include "OnlineSubsystem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include <functional>

EOS_ENABLE_STRICT_WARNINGS

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4071255846, Redpoint::EOS::Identity, IIdentityHook)

class FOnlineSubsystemEOS;
class FOnlineSubsystemRedpointEASFactory;
class FCleanShutdown;
class FEOSLifecycleManager;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineSubsystemRedpointEOSModule : public IModuleInterface, public IOnlineFactory
{
    friend FOnlineSubsystemEOS;
    friend FOnlineSubsystemRedpointEASFactory;
    friend FCleanShutdown;
    friend FEOSLifecycleManager;

private:
    bool IsRegisteredAsSubsystem;
    bool bIsOperatingInNullMode;
    TMap<FName, FOnlineSubsystemEOS *> SubsystemInstances;
    TSharedPtr<Redpoint::EOS::Config::IConfig> EOSConfigInstance;
    TSharedRef<FOnlineSubsystemRedpointEASFactory> EASSubsystemFactory;
    TArray<TSharedRef<Redpoint::EOS::Identity::IIdentityHook>> IdentityHooks;

public:
#if WITH_EDITOR
    TSharedPtr<Redpoint::EOS::Config::IConfig> AutomationTestingConfigOverride;
#endif

    FOnlineSubsystemRedpointEOSModule();
    UE_NONCOPYABLE(FOnlineSubsystemRedpointEOSModule);

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    bool IsOperatingInNullMode() const
    {
        return this->bIsOperatingInNullMode;
    }

    virtual bool SupportsDynamicReloading() override
    {
        return false;
    }

    virtual bool SupportsAutomaticShutdown() override
    {
        return false;
    }

#if !UE_BUILD_SHIPPING
    virtual FString GetPathToEASAutomatedTestingCredentials();
#endif

    bool IsFreeEdition();
    bool HasInstance(FName InstanceName);

    virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName) override;
    IOnlineSubsystemPtr CreateSubsystem(
        FName InstanceName,
        const TSharedRef<Redpoint::EOS::Config::IConfig> &ConfigOverride);

    /**
     * Get an existing instance of the EOS online subsystem. Unlike IOnlineSubsystem::Get(), this function can get
     * existing instances created by automation tests (which use the two parameter version of CreateSubsystem above).
     */
    TSharedPtr<FOnlineSubsystemEOS> GetExistingSubsystem(FName InstanceName);
};

EOS_DISABLE_STRICT_WARNINGS