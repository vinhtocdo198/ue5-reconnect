// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationDiscord/RuntimePlatformIntegrationDiscord.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED
#include "Misc/ConfigCacheIni.h"
#include "RedpointEOSPlatformIntegrationDiscord/Logging.h"
#include "RedpointEOSPlatformIntegrationDiscord/Services/DiscordRuntimePlatformAuthService.h"
#include "RedpointEOSPlatformIntegrationDiscord/Services/DiscordRuntimePlatformPresenceService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2596136541, Redpoint::EOS::Platform::Integration::Discord)
{
using namespace ::Redpoint::EOS::Platform::Types;

FRuntimePlatformIntegrationDiscord::FRuntimePlatformIntegrationDiscord()
    : DiscordClient(MakeShareable(new discordpp::Client()))
    , AuthService(MakeShared<FDiscordRuntimePlatformAuthService>())
    , PresenceService(MakeShared<FDiscordRuntimePlatformPresenceService>(DiscordClient))
{
    this->DiscordClient->AddLogCallback(
        [](std::string Message, discordpp::LoggingSeverity Severity) {
            switch (Severity)
            {
            case discordpp::LoggingSeverity::None:
            case discordpp::LoggingSeverity::Verbose:
            case discordpp::LoggingSeverity::Info:
                UE_LOG(LogRedpointEOSPlatformIntegrationDiscord, Verbose, TEXT("%s"), ANSI_TO_TCHAR(Message.c_str()));
                break;
            case discordpp::LoggingSeverity::Warning:
                UE_LOG(LogRedpointEOSPlatformIntegrationDiscord, Verbose, TEXT("%s"), ANSI_TO_TCHAR(Message.c_str()));
                break;
            case discordpp::LoggingSeverity::Error:
            default:
                UE_LOG(LogRedpointEOSPlatformIntegrationDiscord, Verbose, TEXT("%s"), ANSI_TO_TCHAR(Message.c_str()));
                break;
            }
        },
        discordpp::LoggingSeverity::Verbose);

    auto GetApplicationId = [](const FString &Key) -> TOptional<uint64> {
        FString Str;
        if (GConfig->GetString(TEXT("Redpoint.Discord"), *Key, Str, GEngineIni))
        {
            return FCString::Strtoui64(*Str, nullptr, 10);
        }
        return TOptional<uint64>();
    };

    TOptional<uint64> ApplicationId, SteamApplicationId;
    ApplicationId = GetApplicationId(TEXT("ApplicationId"));
    SteamApplicationId = GetApplicationId(TEXT("SteamApplicationId"));
    if (ApplicationId.IsSet())
    {
        this->DiscordClient->SetApplicationId(ApplicationId.GetValue());
        UE_LOG(
            LogRedpointEOSPlatformIntegrationDiscord,
            Verbose,
            TEXT("Discord Social SDK will use application ID: %llu"),
            ApplicationId.GetValue());

        if (SteamApplicationId.IsSet())
        {
            this->DiscordClient->RegisterLaunchSteamApplication(
                ApplicationId.GetValue(),
                static_cast<uint32>(SteamApplicationId.GetValue()));
            UE_LOG(
                LogRedpointEOSPlatformIntegrationDiscord,
                Verbose,
                TEXT("Discord Social SDK has registered Discord application ID %llu with Steam application ID %u."),
                ApplicationId.GetValue(),
                static_cast<uint32>(SteamApplicationId.GetValue()));
        }
    }
}

TSharedRef<IRuntimePlatformAuthService> FRuntimePlatformIntegrationDiscord::GetAuthService() const
{
    return this->AuthService;
}

TSharedRef<IRuntimePlatformPresenceService> FRuntimePlatformIntegrationDiscord::GetPresenceService() const
{
    return this->PresenceService;
}

void FRuntimePlatformIntegrationDiscord::OnLocalUserSignInAfterSystemStart(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    this->PresenceService->OnLocalUserSignInAfterSystemStart(CallContext);
    OnComplete.ExecuteIfBound();
}

void FRuntimePlatformIntegrationDiscord::OnLocalUserSignOutBeforeSystemStop(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    this->PresenceService->OnLocalUserSignOutBeforeSystemStop(CallContext);
    OnComplete.ExecuteIfBound();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()