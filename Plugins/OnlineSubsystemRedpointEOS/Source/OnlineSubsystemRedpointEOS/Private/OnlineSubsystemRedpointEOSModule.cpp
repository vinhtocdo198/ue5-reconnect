// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Public/OnlineSubsystemRedpointEOSModule.h"

#include "Containers/Ticker.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"
#include "OnlineSubsystemRedpointEOS/Identity/AuthenticationUserInterfaceProviderIdentityHook.h"
#include "OnlineSubsystemRedpointEOS/Identity/CredentialRefreshSubsystemEventIdentityHook.h"
#include "OnlineSubsystemRedpointEOS/Identity/EpicGamesOSSIdentityHook.h"
#include "OnlineSubsystemRedpointEOS/Identity/PlatformUserIdIdentityHook.h"
#include "OnlineSubsystemRedpointEOS/Identity/PrecachePlayerDataStorageFilesIdentityHook.h"
#include "OnlineSubsystemRedpointEOS/Identity/VoiceManagerIdentityHook.h"
#include "OnlineSubsystemRedpointEOS/Legacy/SyntheticSessionManagerIdentityHook.h"
#include "OnlineSubsystemRedpointEOS/Private/NullOSS/OnlineSubsystemRedpointNull.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "OnlineSubsystemRedpointEOS/Shared/EpicGames/OnlineSubsystemRedpointEASFactory.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSConfig/DependentModuleLoader.h"
#include "RedpointEOSConfig/EngineConfigHelpers.h"
#include "RedpointEOSConfig/IniConfig.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"

#if PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"

#include <libloaderapi.h>

#include "Windows/HideWindowsPlatformTypes.h"

#endif

EOS_ENABLE_STRICT_WARNINGS

#define LOCTEXT_NAMESPACE "FOnlineSubsystemRedpointEOSModule"

#if !defined(EOS_SDK_VERSION)
#define EOS_SDK_VERSION "NotSet"
#endif

FOnlineSubsystemRedpointEOSModule::FOnlineSubsystemRedpointEOSModule()
    : IsRegisteredAsSubsystem(false)
    , bIsOperatingInNullMode(false)
    , SubsystemInstances()
    , EOSConfigInstance(nullptr)
    , EASSubsystemFactory(MakeShared<FOnlineSubsystemRedpointEASFactory>(this))
{
}

#if !UE_BUILD_SHIPPING
FString FOnlineSubsystemRedpointEOSModule::GetPathToEASAutomatedTestingCredentials()
{
    return Redpoint::EOS::Core::FModule::GetModuleChecked()
        .GetRuntimePlatform()
        ->GetPathToEASAutomatedTestingCredentials();
}
#endif

bool FOnlineSubsystemRedpointEOSModule::IsFreeEdition()
{
#if defined(EOS_IS_FREE_EDITION)
    return true;
#else
    return false;
#endif
}

bool FOnlineSubsystemRedpointEOSModule::HasInstance(FName InstanceName)
{
    return this->SubsystemInstances.Contains(InstanceName);
}

void FOnlineSubsystemRedpointEOSModule::StartupModule()
{
    if (IsRunningCommandlet() && !FParse::Param(FCommandLine::Get(), TEXT("alloweosincommandlet")))
    {
        // Prevent EOS from loading during cook.
        this->bIsOperatingInNullMode = true;
    }

    checkf(this->IdentityHooks.Num() == 0, TEXT("Expected IdentityHooks to be empty."));
    this->IdentityHooks.Empty();
    {
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity;
        using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy;
        this->IdentityHooks.Add(MakeShared<FAuthenticationUserInterfaceProviderIdentityHook>());
        this->IdentityHooks.Add(MakeShared<FCredentialRefreshSubsystemEventIdentityHook>());
        this->IdentityHooks.Add(MakeShared<FPlatformUserIdIdentityHook>());
        this->IdentityHooks.Add(MakeShared<FVoiceManagerIdentityHook>());
        this->IdentityHooks.Add(MakeShared<FEpicGamesOSSIdentityHook>());
        this->IdentityHooks.Add(MakeShared<FSyntheticSessionManagerIdentityHook>());
        this->IdentityHooks.Add(MakeShared<FPrecachePlayerDataStorageFilesIdentityHook>());
#endif
    }
    for (const auto &Hook : this->IdentityHooks)
    {
        Redpoint::EOS::Identity::FIdentityHookRegistry::Register(Hook, 90000);
    }

#if defined(EOS_IS_FREE_EDITION)
    UE_LOG(
        LogRedpointEOS,
        Warning,
        TEXT("This application is using the Free Edition of the Redpoint EOS Online Framework plugin. The developer of this "
             "application must not be earning more than $30,000 USD in a calendar year. See "
             "https://redpoint.games/eos-online-framework-free-eula/ for the Redpoint EOS Online Framework Free Edition license "
             "agreement. If you have inquiries about this notice, you can email sales@redpoint.games."));
#endif

    FModuleManager &ModuleManager = FModuleManager::Get();

#if REDPOINT_EOS_UE_5_5_OR_LATER
    if (ModuleManager.GetModule("OnlineSubsystemEOS") != nullptr)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("The Epic \"Online Subsystem EOS\" plugin is enabled in this project. This plugin conflicts with the "
                 "Redpoint EOS plugin, and it must be disabled from the Plugins window (with the editor restarted "
                 "afterwards) before the Redpoint EOS plugin will work."));
        return;
    }
#else
    if (ModuleManager.GetModule("OnlineSubsystemEOS") != nullptr || ModuleManager.GetModule("EOSShared") != nullptr)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("The Epic \"Online Subsystem EOS\" or \"EOS Shared\" plugin is enabled in this project. These "
                 "plugins conflict with the Redpoint EOS plugin, and they must be disabled from the Plugins window "
                 "(with the editor restarted afterwards) before the Redpoint EOS plugin will work."));
        return;
    }
#endif

    // Add the DefaultOnlineSubsystemRedpointEOS.ini file in the project's Config directory to the static
    // INI hierarchy of OnlineSubsystemRedpointEOS.ini at the correct location in the hierarchy.
    {
        FString Filename;
        FConfigCacheIni::LoadGlobalIniFile(Filename, TEXT("OnlineSubsystemRedpointEOS"));
        if (!Filename.IsEmpty())
        {
#if REDPOINT_EOS_UE_5_5_OR_LATER
            FConfigBranch *Branch = GConfig->FindBranch(FName("OnlineSubsystemRedpointEOS"), "");
            if (Branch != nullptr)
            {
                Branch->AddDynamicLayerToHierarchy(
                    FConfigCacheIni::NormalizeConfigIniPath(FPaths::SourceConfigDir()) /
                        "DefaultOnlineSubsystemRedpointEOS.ini",
                    nullptr);
            }
            else
            {
                UE_LOG(
                    LogRedpointEOS,
                    Warning,
                    TEXT("Unable to apply project-specific overrides to OnlineSubsystemRedpointEOS.ini; custom net "
                         "driver settings will not be loaded."));
            }
#else
            FConfigFile *File = Redpoint::EOS::Config::FEngineConfigHelpers::FindByName(Filename);
            if (File != nullptr)
            {
                File->AddDynamicLayerToHierarchy(
                    FConfigCacheIni::NormalizeConfigIniPath(FPaths::SourceConfigDir()) /
                    "DefaultOnlineSubsystemRedpointEOS.ini");
            }
            else
            {
                UE_LOG(
                    LogRedpointEOS,
                    Warning,
                    TEXT("Unable to apply project-specific overrides to OnlineSubsystemRedpointEOS.ini; custom net "
                         "driver settings will not be loaded."));
            }
#endif
        }
        else
        {
            UE_LOG(
                LogRedpointEOS,
                Warning,
                TEXT("Unable to determine config path for OnlineSubsystemRedpointEOS.ini; custom net "
                     "driver settings will not be loaded."));
        }
    }

    check(!this->EOSConfigInstance.IsValid());
    this->EOSConfigInstance = MakeShared<Redpoint::EOS::Config::FIniConfig>();

    if (this->bIsOperatingInNullMode)
    {
        UE_LOG(
            LogRedpointEOS,
            Verbose,
            TEXT("EOS module is operating in NULL mode, since this is a commandlet. Most functionality will be "
                 "unavailable."));

        // Register the subsystem (this will generate null instances).

        auto OSS = ModuleManager.GetModule("OnlineSubsystem");
        if (OSS != nullptr)
        {
            ((FOnlineSubsystemModule *)OSS)->RegisterPlatformService(REDPOINT_EOS_SUBSYSTEM, this);
            this->IsRegisteredAsSubsystem = true;
        }
    }
    else
    {
        Redpoint::EOS::Config::FDependentModuleLoader::LoadConfigDependentModules(*this->EOSConfigInstance.Get());
        Redpoint::EOS::Config::FDependentModuleLoader::LoadPlatformDependentModules();

#if EOS_HAS_AUTHENTICATION
        // Register all the built-in authentication graphs. If you want to register your own authentication graphs, you
        // can do so inside your game's StartupModule.
        Redpoint::EOS::Auth::FAuthenticationGraphRegistry::RegisterDefaults();
#endif // #if EOS_HAS_AUTHENTICATION

        // Register the subsystem.
        auto OSS = ModuleManager.GetModule("OnlineSubsystem");
        if (OSS != nullptr)
        {
            ((FOnlineSubsystemModule *)OSS)->RegisterPlatformService(REDPOINT_EOS_SUBSYSTEM, this);
#if EOS_HAS_AUTHENTICATION
            ((FOnlineSubsystemModule *)OSS)
                ->RegisterPlatformService(REDPOINT_EAS_SUBSYSTEM, &this->EASSubsystemFactory.Get());
#endif
            this->IsRegisteredAsSubsystem = true;
        }

        UE_LOG(LogRedpointEOS, Verbose, TEXT("EOS module has finished starting up."));

#if WITH_EDITOR
        FWorldDelegates::OnPostWorldInitialization.AddLambda([](UWorld *World, const UWorld::InitializationValues IVS) {
            if (FParse::Param(FCommandLine::Get(), TEXT("eosauthorizer")))
            {
                UE_LOG(
                    LogRedpointEOS,
                    Verbose,
                    TEXT("Starting automatic editor-based authorization due to -eosauthorizer flag."));
                IOnlineSubsystem *OSS = Online::GetSubsystem(World, REDPOINT_EOS_SUBSYSTEM);
                if (OSS != nullptr)
                {
                    IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
                    Identity->AddOnLoginCompleteDelegate_Handle(
                        0,
                        FOnLoginCompleteDelegate::CreateLambda([](int32 LocalUserNum,
                                                                  bool bWasSuccessful,
                                                                  const FUniqueNetId &UserId,
                                                                  const FString &Error) {
                            if (!Error.IsEmpty() && !bWasSuccessful)
                            {
                                UE_LOG(LogRedpointEOS, Error, TEXT("The error from authentication was: %s"), *Error);
                            }

                            // Delaying this by one frame prevents a crash if the AutoLogin fails immediately (e.g. if
                            // the Free Edition user hasn't set a license key).
                            FTSTicker::GetCoreTicker().AddTicker(
                                FTickerDelegate::CreateLambda([](float DeltaSeconds) {
                                    FGenericPlatformMisc::RequestExit(false);
                                    return false;
                                }),
                                0.0f);
                        }));
                    Identity->AutoLogin(0);
                }
            }
        });
#endif
    }
}

IOnlineSubsystemPtr FOnlineSubsystemRedpointEOSModule::CreateSubsystem(FName InstanceName)
{
#if WITH_EDITOR
    if (GIsAutomationTesting)
    {
        // Automation testing, but not through a unit test. This scenario is hit when testing the networking with
        // play-in-editor.
        if (this->AutomationTestingConfigOverride.IsValid())
        {
            return this->CreateSubsystem(InstanceName, this->AutomationTestingConfigOverride.ToSharedRef());
        }
    }
#endif

    return this->CreateSubsystem(InstanceName, this->EOSConfigInstance.ToSharedRef());
}

IOnlineSubsystemPtr FOnlineSubsystemRedpointEOSModule::CreateSubsystem(
    FName InstanceName,
    const TSharedRef<Redpoint::EOS::Config::IConfig> &ConfigOverride)
{
    if (this->bIsOperatingInNullMode)
    {
        return MakeShared<FOnlineSubsystemRedpointNull, ESPMode::ThreadSafe>(InstanceName);
    }
    else
    {
        check(!SubsystemInstances.Contains(InstanceName));

        auto Inst = MakeShared<FOnlineSubsystemEOS, ESPMode::ThreadSafe>(InstanceName, this, ConfigOverride);
        if (Inst->IsEnabled())
        {
            if (!Inst->Init())
            {
                UE_LOG(LogRedpointEOS, Warning, TEXT("Unable to init online subsystem for Epic Online Services."));
                Inst->Shutdown();

#if REDPOINT_EOS_IS_DEDICATED_SERVER
                FPlatformMisc::RequestExit(true);
#endif

                return nullptr;
            }
        }
        else
        {
            UE_LOG(LogRedpointEOS, Warning, TEXT("The online subsystem provided by Redpoint EOS Online Framework is not enabled."));
            Inst->Shutdown();

#if REDPOINT_EOS_IS_DEDICATED_SERVER
            FPlatformMisc::RequestExit(true);
#endif
            return nullptr;
        }

        return Inst;
    }
}

void FOnlineSubsystemRedpointEOSModule::ShutdownModule()
{
    UE_LOG(LogRedpointEOS, Verbose, TEXT("EOS module shutting down."));

    TArray<class FOnlineSubsystemEOS *> Arr;
    for (auto Subsystem : this->SubsystemInstances)
    {
        Arr.Add(Subsystem.Value);
    }
    for (auto Subsystem : Arr)
    {
        bool bStillHasSubsystem = false;
        for (auto S : this->SubsystemInstances)
        {
            if (S.Value == Subsystem)
            {
                bStillHasSubsystem = true;
            }
        }
        if (bStillHasSubsystem)
        {
            Subsystem->Shutdown();
        }
    }
    int NonShuttingDownInstances = 0;
    for (auto Subsystem : this->SubsystemInstances)
    {
        if (!Subsystem.Key.ToString().EndsWith(TEXT("_ShuttingDown")))
        {
            NonShuttingDownInstances++;
        }
    }
    ensureMsgf(
        NonShuttingDownInstances == 0,
        TEXT("Online subsystems did not cleanly shutdown on module shutdown. Ensure you are not holding references "
             "to online subsystem objects longer than the lifetime of the online subsystem."));

    if (this->IsRegisteredAsSubsystem)
    {
        FModuleManager &ModuleManager = FModuleManager::Get();
        auto OSS = ModuleManager.GetModule("OnlineSubsystem");
        if (OSS != nullptr)
        {
            ((FOnlineSubsystemModule *)OSS)->UnregisterPlatformService(REDPOINT_EAS_SUBSYSTEM);
            ((FOnlineSubsystemModule *)OSS)->UnregisterPlatformService(REDPOINT_EOS_SUBSYSTEM);
            this->IsRegisteredAsSubsystem = false;
        }
    }

    for (const auto &Hook : this->IdentityHooks)
    {
        Redpoint::EOS::Identity::FIdentityHookRegistry::Unregister(Hook);
    }
    this->IdentityHooks.Empty();

    UE_LOG(LogRedpointEOS, Verbose, TEXT("EOS module shutdown complete."));

    this->EOSConfigInstance.Reset();
}

TSharedPtr<FOnlineSubsystemEOS> FOnlineSubsystemRedpointEOSModule::GetExistingSubsystem(FName InstanceName)
{
    auto *ExistingInstance = this->SubsystemInstances.Find(InstanceName);
    if (ExistingInstance != nullptr)
    {
        return (*ExistingInstance)->AsShared();
    }
    return nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOnlineSubsystemRedpointEOSModule, OnlineSubsystemRedpointEOS)

EOS_DISABLE_STRICT_WARNINGS
