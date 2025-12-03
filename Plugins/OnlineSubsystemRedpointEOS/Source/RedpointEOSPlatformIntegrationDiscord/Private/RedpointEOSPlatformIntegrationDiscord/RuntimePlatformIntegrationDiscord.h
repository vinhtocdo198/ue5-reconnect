// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatformIntegrationDiscord/Services/DiscordRuntimePlatformPresenceService.h"
#include "discordpp.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2596136541, Redpoint::EOS::Platform::Integration::Discord)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Integration::Discord;
using namespace ::Redpoint::EOS::Platform::Services;

class FRuntimePlatformIntegrationDiscord : public IRuntimePlatformIntegration
{
private:
    TSharedRef<discordpp::Client> DiscordClient;
    TSharedRef<IRuntimePlatformAuthService> AuthService;
    TSharedRef<FDiscordRuntimePlatformPresenceService> PresenceService;

public:
    FRuntimePlatformIntegrationDiscord();
    UE_NONCOPYABLE(FRuntimePlatformIntegrationDiscord);
    virtual ~FRuntimePlatformIntegrationDiscord() override = default;

    virtual TSharedRef<IRuntimePlatformAuthService> GetAuthService() const override;
    virtual TSharedRef<IRuntimePlatformPresenceService> GetPresenceService() const override;

    virtual void OnLocalUserSignInAfterSystemStart(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override;
    virtual void OnLocalUserSignOutBeforeSystemStop(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override;
};

}

namespace Redpoint::EOS::Platform::Integration::Discord
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2596136541,
    Redpoint::EOS::Platform::Integration::Discord,
    FRuntimePlatformIntegrationDiscord)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()