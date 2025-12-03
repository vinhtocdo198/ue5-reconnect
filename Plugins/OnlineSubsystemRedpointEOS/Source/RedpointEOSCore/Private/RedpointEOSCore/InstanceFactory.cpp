// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/InstanceFactory.h"

#include "Engine/Engine.h"
#include "Misc/CommandLine.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSAPI/Platform/Create.h"
#include "RedpointEOSConfig/DependentModuleLoader.h"
#include "RedpointEOSCore/Logging.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatform/LifecycleManager.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2644256279, Redpoint::EOS::Core)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::API;

bool FInstanceFactory::IsTrueDedicated(const FName &InInstanceName)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    // Neither IsServer nor IsDedicated work correctly in play-in-editor. Both listen servers
    // and dedicated servers return true from IsServer, but *neither* return true from IsDedicated
    // (in fact it looks like IsDedicated just doesn't do the right thing at all for single process).
    //
    // So instead implement our own version here which does the detection correctly.

    if (GIsAutomationTesting && InInstanceName.ToString().EndsWith(TEXT("_AutomationDedicatedServer")))
    {
        // Allow automation tests to force creation of dedicated server instance.
        return true;
    }

#if WITH_EDITOR
    // Running multiple worlds in the editor; we need to see if this world is specifically a dedicated server.
    FName WorldContextHandle =
        (InInstanceName != NAME_None && InInstanceName != TEXT("DefaultInstance")) ? InInstanceName : NAME_None;
    if (WorldContextHandle.IsNone())
    {
        // The default OSS instance is equal to IsRunningDedicatedServer(), in case the editor
        // is being run with -server for multi-process.
        return IsRunningDedicatedServer();
    }
    if (GEngine == nullptr)
    {
        UE_LOG(
            LogRedpointEOSCore,
            Error,
            TEXT("GEngine is not available, but EOS requires it in order to detect if it is running as a dedicated "
                 "server inside the editor! Please report this error in the Redpoint Developer Support Discord!"));
        return false;
    }
    FWorldContext *WorldContext = GEngine->GetWorldContextFromHandle(WorldContextHandle);
    if (WorldContext == nullptr)
    {
        // World context is invalid. This will occur during unit tests.
        return false;
    }
    return WorldContext->RunAsDedicated;
#else
    // Just use IsRunningDedicatedServer, since our process is either a server or it's not.
    return IsRunningDedicatedServer();
#endif
}

FPlatformHandle FInstanceFactory::Create(const FPlatformInstanceEnvironmentRef &InEnvironment)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    using namespace ::Redpoint::EOS::API::Platform;

    // Load modules that are we are dependent on for delegated subsystems.
    Config::FDependentModuleLoader::LoadConfigDependentModules(*InEnvironment->Config);
    Config::FDependentModuleLoader::LoadPlatformDependentModules();

    // Get the runtime platform.
    auto RuntimePlatform = FModule::GetModuleChecked().GetRuntimePlatform();

    // Figure out the flags to apply when creating the platform.
    uint64_t Flags = 0;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (!InEnvironment->Config->GetCrossPlatformAccountProvider().IsEqual("EpicGames") &&
        FString(FCommandLine::Get()).ToUpper().Contains(TEXT("-EPICPORTAL")))
    {
        // Only enable if we can ever authenticate with Epic Games, otherwise it'll never appear anyway.
        Flags = EOS_PF_DISABLE_OVERLAY;
    }
    if (InEnvironment->Config->GetNeverShowEpicGamesOverlay())
    {
        // If the developer has explicitly requested that the overlay should always be disabled, turn it off.
        Flags = EOS_PF_DISABLE_OVERLAY;
    }
#if WITH_EDITOR
    else if (!FString(FCommandLine::Get()).ToUpper().Contains(TEXT("-GAME")))
    {
        // Disable the overlay in editor builds, if we're not running as a standalone process.
        Flags = EOS_PF_LOADING_IN_EDITOR;
    }
#endif // #if WITH_EDITOR
#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // Figure out the sandbox and deployment ID to use for the platform.
    FString SandboxId = InEnvironment->Config->GetSandboxId();
    FString DeploymentId = InEnvironment->Config->GetDeploymentId();
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (FString(FCommandLine::Get()).ToUpper().Contains(TEXT("-EPICPORTAL")))
    {
        // When launching through EGS, use the sandbox and deployment IDs from the command line.
        FString CandidateSandboxId, CandidateDeploymentId;
        if (FParse::Value(FCommandLine::Get(), TEXT("epicsandboxid="), CandidateSandboxId, true) &&
            FParse::Value(FCommandLine::Get(), TEXT("epicdeploymentid="), CandidateDeploymentId, true))
        {
            UE_LOG(
                LogRedpointEOSCore,
                Verbose,
                TEXT("Using sandbox ID (%s) and deployment ID (%s) provided on command line by Epic Games Launcher."),
                *CandidateSandboxId,
                *CandidateDeploymentId);
            SandboxId = CandidateSandboxId;
            DeploymentId = CandidateDeploymentId;
        }
        else
        {
            UE_LOG(
                LogRedpointEOSCore,
                Warning,
                TEXT("Detected -EPICPORTAL on command line, but missing -epicsandboxid= or -epicdeploymentid= values. "
                     "This game will use the Live sandbox and deployment IDs previously provided in configuration."));
        }
    }
#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // Finalise the options for platform creation.
    FCreate::Options Options = {
        InEnvironment->bIsDedicatedServer,
        InEnvironment->Config->GetEncryptionKey(),
        RuntimePlatform->GetCacheDirectory(),
        TEXT(""),
        TEXT(""),
        Flags,
        InEnvironment->Config->GetProductId(),
        SandboxId,
        DeploymentId,
        InEnvironment->bIsDedicatedServer ? InEnvironment->Config->GetDedicatedServerClientId()
                                          : InEnvironment->Config->GetClientId(),
        InEnvironment->bIsDedicatedServer ? InEnvironment->Config->GetDedicatedServerClientSecret()
                                          : InEnvironment->Config->GetClientSecret(),
        RuntimePlatform->GetRTCOptions(),
        0u,
        InEnvironment->Config->GetReservedOptions(),
    };
    UE_LOG(
        LogRedpointEOSCore,
        Verbose,
        TEXT("Creating EOS SDK platform instance: InstanceName=%s, bIsServer=%s"),
        *InEnvironment->InstanceName.ToString(),
        InEnvironment->bIsDedicatedServer ? TEXT("true") : TEXT("false"));

    // Create the platform.
    EOS_HPlatform RawPlatformHandle = FCreate::Execute(Options);
    if (RawPlatformHandle == nullptr)
    {
        UE_LOG(LogRedpointEOSCore, Error, TEXT("Unable to initialize EOS platform."));
        if (FParse::Param(FCommandLine::Get(), TEXT("requireeos")))
        {
            checkf(
                RawPlatformHandle != nullptr,
                TEXT("Unable to initialize EOS platform and you have -REQUIREEOS on the command line."));
        }
        return API::FPlatformInstance::CreateDeadWithNoInstance(InEnvironment);
    }
    auto PlatformHandle = MakeShared<Redpoint::EOS::API::FPlatformInstance>(RawPlatformHandle, InEnvironment);

    UE_LOG(LogRedpointEOSCore, Verbose, TEXT("EOS platform %p has been created."), RawPlatformHandle);

    // Set the initial application and network status from the runtime platform.
    RuntimePlatform->SetLifecycleForNewPlatformInstance(PlatformHandle);

    return PlatformHandle;
}

#if REDPOINT_EOS_UE_5_5_OR_LATER && WITH_EDITOR

FPlatformHandle FInstanceFactory::CreateFromRawOptions(
    const FPlatformInstanceEnvironmentRef &InEnvironment,
    EOS_Platform_Options &PlatformOptions)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    using namespace ::Redpoint::EOS::API::Platform;

    // Finalise the options for platform creation.
    FCreate::Options Options = {
        PlatformOptions.bIsServer == EOS_TRUE,
        PlatformOptions.EncryptionKey == nullptr ? TEXT("") : ANSI_TO_TCHAR(PlatformOptions.EncryptionKey),
        PlatformOptions.CacheDirectory == nullptr ? TEXT("") : ANSI_TO_TCHAR(PlatformOptions.CacheDirectory),
        PlatformOptions.OverrideCountryCode == nullptr ? TEXT("") : ANSI_TO_TCHAR(PlatformOptions.OverrideCountryCode),
        PlatformOptions.OverrideLocaleCode == nullptr ? TEXT("") : ANSI_TO_TCHAR(PlatformOptions.OverrideLocaleCode),
        PlatformOptions.Flags,
        PlatformOptions.ProductId == nullptr ? TEXT("") : ANSI_TO_TCHAR(PlatformOptions.ProductId),
        PlatformOptions.SandboxId == nullptr ? TEXT("") : ANSI_TO_TCHAR(PlatformOptions.SandboxId),
        PlatformOptions.DeploymentId == nullptr ? TEXT("") : ANSI_TO_TCHAR(PlatformOptions.DeploymentId),
        PlatformOptions.ClientCredentials.ClientId == nullptr
            ? TEXT("")
            : ANSI_TO_TCHAR(PlatformOptions.ClientCredentials.ClientId),
        PlatformOptions.ClientCredentials.ClientSecret == nullptr
            ? TEXT("")
            : ANSI_TO_TCHAR(PlatformOptions.ClientCredentials.ClientSecret),
        const_cast<EOS_Platform_RTCOptions *>(PlatformOptions.RTCOptions),
        PlatformOptions.TickBudgetInMilliseconds,
        PlatformOptions.Reserved,
    };
    UE_LOG(
        LogRedpointEOSCore,
        Verbose,
        TEXT("Creating EOS SDK platform instance (FOR FAB): InstanceName=%s"),
        *InEnvironment->InstanceName.ToString());

    // Create the platform.
    EOS_HPlatform RawPlatformHandle = FCreate::Execute(Options);
    if (RawPlatformHandle == nullptr)
    {
        UE_LOG(LogRedpointEOSCore, Error, TEXT("Unable to initialize EOS platform (FOR FAB)."));
        return API::FPlatformInstance::CreateDeadWithNoInstance(InEnvironment);
    }
    auto PlatformHandle = MakeShared<Redpoint::EOS::API::FPlatformInstance>(RawPlatformHandle, InEnvironment);

    UE_LOG(LogRedpointEOSCore, Verbose, TEXT("EOS platform (FOR FAB) %p has been created."), RawPlatformHandle);

    return PlatformHandle;
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()