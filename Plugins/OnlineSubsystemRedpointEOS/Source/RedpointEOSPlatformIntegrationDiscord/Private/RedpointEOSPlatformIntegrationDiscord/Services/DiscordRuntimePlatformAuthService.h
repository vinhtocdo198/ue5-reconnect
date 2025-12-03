// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1835581414, Redpoint::EOS::Platform::Integration::Discord)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Platform::Services;

class FDiscordRuntimePlatformAuthService : public IRuntimePlatformAuthService
{
public:
    FDiscordRuntimePlatformAuthService() = default;
    UE_NONCOPYABLE(FDiscordRuntimePlatformAuthService);
    virtual ~FDiscordRuntimePlatformAuthService() override = default;

    virtual FName GetPlatformName() const override;
    virtual int32 GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const override;
    virtual void EnsureSignedInOnLocalPlatform(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnEnsureSignedInOnLocalPlatform &OnComplete) override;
    virtual void IsOnlinePlayPermitted(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnIsOnlinePlayPermitted &OnComplete) override;
    virtual void GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const TSharedRef<FAuthenticationGraphState> &State,
        const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete) override;
};

}

namespace Redpoint::EOS::Platform::Integration::Discord
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1835581414,
    Redpoint::EOS::Platform::Integration::Discord,
    FDiscordRuntimePlatformAuthService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()