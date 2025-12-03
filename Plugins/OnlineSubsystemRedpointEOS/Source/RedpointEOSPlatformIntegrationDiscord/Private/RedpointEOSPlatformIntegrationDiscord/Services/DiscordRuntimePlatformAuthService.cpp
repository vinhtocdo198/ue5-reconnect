// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationDiscord/Services/DiscordRuntimePlatformAuthService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1835581414, Redpoint::EOS::Platform::Integration::Discord)
{
using namespace ::Redpoint::EOS::API;

FName FDiscordRuntimePlatformAuthService::GetPlatformName() const
{
    return TEXT("Discord");
}

int32 FDiscordRuntimePlatformAuthService::GetSelectionScore(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    // Discord can no longer be used as a platform for authentication, as platform authentication is intended to
    // authenticate Epic Online Services with the current store. Discord no longer has a store, and can't launch games
    // that players own, so this type of flow doesn't make sense (and the new Social SDK does not provide suitable APIs
    // for detecting if Discord *could* successfully perform authentication).
    //
    // With the new Discord Social SDK, Discord would be a better fit as an alternative cross-platform account provider,
    // whereby we get the platform token and provide it to Discord for it's "external auth" flows. We could then chain
    // the Discord OAuth2 token from it's cross-platform account provider into EOS using Discord auth.
    return -10000;
}

void FDiscordRuntimePlatformAuthService::EnsureSignedInOnLocalPlatform(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnEnsureSignedInOnLocalPlatform &OnComplete)
{
    OnComplete.ExecuteIfBound(Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("This function should not be called on the Discord auth service.")));
}

void FDiscordRuntimePlatformAuthService::IsOnlinePlayPermitted(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnIsOnlinePlayPermitted &OnComplete)
{
    OnComplete.ExecuteIfBound(Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("This function should not be called on the Discord auth service.")));
}

void FDiscordRuntimePlatformAuthService::GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const TSharedRef<FAuthenticationGraphState> &State,
    const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete)
{
    OnComplete.ExecuteIfBound(Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("This function should not be called on the Discord auth service.")));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()