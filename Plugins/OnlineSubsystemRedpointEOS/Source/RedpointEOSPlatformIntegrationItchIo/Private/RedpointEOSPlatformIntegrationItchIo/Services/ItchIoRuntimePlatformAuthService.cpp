// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationItchIo/Services/ItchIoRuntimePlatformAuthService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSPlatformIntegrationItchIo/Auth/ItchIoExternalCredentials.h"
#include "RedpointEOSPlatformIntegrationItchIo/ItchIoSystem.h"
#include "RedpointEOSPlatformIntegrationItchIo/Logging.h"
#include "RedpointEOSPlatformIntegrationItchIo/PlatformName.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3491869548, Redpoint::EOS::Platform::Integration::ItchIo::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::ItchIo;
using namespace ::Redpoint::EOS::Platform::Integration::ItchIo::Auth;

FName FItchIoRuntimePlatformAuthService::GetPlatformName() const
{
    return PlatformNameItchIo;
}

int32 FItchIoRuntimePlatformAuthService::GetSelectionScore(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    FString ItchIoApiKey = FPlatformMisc::GetEnvironmentVariable(TEXT("ITCHIO_API_KEY"));
    if (!ItchIoApiKey.IsEmpty())
    {
        return 1000;
    }
    return 0;
}

void FItchIoRuntimePlatformAuthService::EnsureSignedInOnLocalPlatform(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnEnsureSignedInOnLocalPlatform &OnComplete)
{
    AsCallback(
        CallContext->PlatformHandle->GetSystem<FItchIoSystem>()->TryLoginLocalUserFromApiKey(),
        [OnComplete](FError Result) {
            OnComplete.ExecuteIfBound(Result);
        });
}

void FItchIoRuntimePlatformAuthService::IsOnlinePlayPermitted(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnIsOnlinePlayPermitted &OnComplete)
{
    OnComplete.ExecuteIfBound(Errors::Success());
}

void FItchIoRuntimePlatformAuthService::GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const TSharedRef<FAuthenticationGraphState> &State,
    const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete)
{
    auto LocalUser = CallContext->PlatformHandle->GetSystem<FItchIoSystem>()->GetLocalUser();

    if (!LocalUser.IsSet())
    {
        OnComplete.ExecuteIfBound(
            Errors::AccessDenied(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user is not signed into itch.io.")));
        return;
    }

    FString ApiKey = FPlatformMisc::GetEnvironmentVariable(TEXT("ITCHIO_API_KEY"));

    TMap<FString, FString> UserAuthAttributes;
    UserAuthAttributes.Add(EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH, TEXT("itchIo"));
    UserAuthAttributes.Add(TEXT("itchIo.apiKey"), ApiKey);

    UE_LOG(
        LogRedpointEOSPlatformIntegrationItchIo,
        Verbose,
        TEXT("Using itch.io API key provided via the ITCHIO_API_KEY environment variable."));
    State->AvailableExternalCredentials.Add(MakeShared<FItchIoExternalCredentials>(ApiKey, UserAuthAttributes));
    OnComplete.ExecuteIfBound(Errors::Success());
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()