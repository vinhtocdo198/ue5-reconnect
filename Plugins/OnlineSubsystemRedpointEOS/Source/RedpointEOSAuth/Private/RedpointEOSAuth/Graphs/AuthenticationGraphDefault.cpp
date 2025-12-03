// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphDefault.h"

#include "Engine/World.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphAlwaysFail.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphCrossPlatformOnly.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphDevAuthTool.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphEpicGamesExchangeCode.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphRuntimePlatform.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1044168076, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Auth;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

FName FAuthenticationGraphDefault::Resolve(
    const TSharedRef<FAuthenticationGraphRegistry> &InRegistry,
    const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig,
    const FOnlineProvidedCredentials &InProvidedCredentials,
    const TSoftObjectPtr<UWorld> &InWorld,
    const FPlatformHandle &InPlatformHandle,
    int32 InLocalUserNum)
{
    if (InRegistry->Has(FName(PREPROCESSOR_TO_STRING(PLATFORM_HEADER_NAME))))
    {
        // Pick the platform specific subsystem if there is one.
        return FName(PREPROCESSOR_TO_STRING(PLATFORM_HEADER_NAME));
    }

    // Otherwise we need to choose the graph, based on what online subsystem is available.

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    {
        bool bIgnoredParam;
        if (FParse::Bool(FCommandLine::Get(), TEXT("EpicPortal"), bIgnoredParam) &&
            InRegistry->Has(EOS_AUTH_GRAPH_EPIC_GAMES_EXCHANGE_CODE))
        {
            FString AuthType;
            FString ExchangeCode;
            if (FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType, true) &&
                AuthType == TEXT("exchangecode") &&
                FParse::Value(FCommandLine::Get(), TEXT("AUTH_PASSWORD="), ExchangeCode, true))
            {
                // Forces the use of Epic Games as a cross-platform account provider when we're launched on the Epic
                // Games store, regardless of the configured cross-platform account provider. This allows developers to
                // ship games on both Steam and EGS without ever prompting users to sign in or create accounts on Steam
                // (as would happen with optional cross-platform accounts).
                return EOS_AUTH_GRAPH_EPIC_GAMES_EXCHANGE_CODE;
            }
        }
    }
#endif

    auto AuthService =
        FAuthenticationGraphState::GetAuthServiceStatic(InPlatformHandle, InConfig, InWorld, InLocalUserNum);
    if (AuthService.IsValid())
    {
        return EOS_AUTH_GRAPH_RUNTIME_PLATFORM;
    }

#if WITH_EDITOR && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)
    // @note: This path is *NOT* what is used for "Login before PIE" option. Instead, refer to FIdentitySystemGameImpl
    // which forces selection of the Dev Auth Tool if the provided credentials have 'DEV_TOOL_AUTO_LOGIN' as the ID.
    if (InWorld.IsValid() && InRegistry->Has(EOS_AUTH_GRAPH_DEV_AUTH_TOOL) &&
        (InWorld->WorldType == EWorldType::Editor || InWorld->WorldType == EWorldType::PIE ||
         InWorld->WorldType == EWorldType::EditorPreview || InWorld->WorldType == EWorldType::Inactive ||
         /* Standalone game */
         InWorld->IsPlayInPreview()))
    {
        UE_LOG(
            LogRedpointEOSAuth,
            Verbose,
            TEXT("Choosing to authenticate via the Developer Authentication Tool because you are running in the editor "
                 "and there is no other way to authenticate."));
        return EOS_AUTH_GRAPH_DEV_AUTH_TOOL;
    }
#endif

    return EOS_AUTH_GRAPH_ALWAYS_FAIL;
}

FName FAuthenticationGraphDefault::ResolveWithCrossPlatform(
    const TSharedRef<FAuthenticationGraphRegistry> &InRegistry,
    const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig,
    const FOnlineProvidedCredentials &InProvidedCredentials,
    const TSoftObjectPtr<UWorld> &InWorld,
    const FPlatformHandle &InPlatformHandle,
    int32 InLocalUserNum)
{
    FName Result = FAuthenticationGraphDefault::Resolve(
        InRegistry,
        InConfig,
        InProvidedCredentials,
        InWorld,
        InPlatformHandle,
        InLocalUserNum);
    if (Result == EOS_AUTH_GRAPH_ALWAYS_FAIL)
    {
        if (!InConfig->GetCrossPlatformAccountProvider().IsNone())
        {
            return EOS_AUTH_GRAPH_CROSS_PLATFORM_ONLY;
        }
    }
    return Result;
}

void FAuthenticationGraphDefault::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_DEFAULT,
        NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_Default", "Default"),
        FResolveAuthenticationGraphDelegate::CreateStatic(&FAuthenticationGraphDefault::Resolve));
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_DEFAULT_WITH_CROSS_PLATFORM_FALLBACK,
        NSLOCTEXT(
            "OnlineSubsystemRedpointEOS",
            "AuthGraph_DefaultCrossPlatformFallback",
            "Default, with Cross-Platform Fallback"),
        FResolveAuthenticationGraphDelegate::CreateStatic(&FAuthenticationGraphDefault::ResolveWithCrossPlatform));
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()