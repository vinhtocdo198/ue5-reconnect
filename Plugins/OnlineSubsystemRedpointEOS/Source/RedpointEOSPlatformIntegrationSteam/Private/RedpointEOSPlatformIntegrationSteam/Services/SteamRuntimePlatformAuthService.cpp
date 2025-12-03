// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationSteam/Services/SteamRuntimePlatformAuthService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialInfo.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialObtainer.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamExternalCredentials.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3144435893, Redpoint::EOS::Platform::Integration::Steam::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::Steam::Auth;

FSteamRuntimePlatformAuthService::FSteamRuntimePlatformAuthService()
    : FOSSv1RuntimePlatformAuthService(
          FName(TEXT("Steam")),
          EOS_EExternalCredentialType::EOS_ECT_STEAM_SESSION_TICKET,
          TEXT("steam"),
          TEXT("steam.sessionTicket"))
{
}

int32 FSteamRuntimePlatformAuthService::GetSelectionScore(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    IOnlineSubsystem *SteamOSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (SteamOSS != nullptr)
    {
        IOnlineIdentityPtr SteamIdentity = SteamOSS->GetIdentityInterface();
        if (SteamIdentity.IsValid() && SteamIdentity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
        {
            return 1000;
        }
    }
    return 0;
}

void FSteamRuntimePlatformAuthService::GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const TSharedRef<FAuthenticationGraphState> &State,
    const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete)
{
    FSteamCredentialObtainer::StartFromAuthenticationGraph(
        State,
        FSteamCredentialObtainer::FOnCredentialObtained::CreateSPLambda(
            this,
            [State, OnComplete](const FError &ErrorCode, FSteamCredentialInfo ObtainedCredentials) {
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(ErrorCode);
                    return;
                }

                State->AvailableExternalCredentials.Add(MakeShared<FSteamExternalCredentials>(ObtainedCredentials));
                OnComplete.ExecuteIfBound(Errors::Success());
            }));
}

};

#endif

REDPOINT_EOS_CODE_GUARD_END()