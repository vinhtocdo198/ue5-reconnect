// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamExternalCredentials.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "OnlineSubsystemNames.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialObtainer.h"

namespace REDPOINT_EOS_FILE_NS_ID(4063608760, Redpoint::EOS::Platform::Integration::Steam::Auth)
{

FSteamExternalCredentials::FSteamExternalCredentials(const FSteamCredentialInfo &InCredentialInfo)
    : CredentialInfo(InCredentialInfo)
{
}

FText FSteamExternalCredentials::GetProviderDisplayName() const
{
    return NSLOCTEXT("OnlineSubsystemEOSAuthSteam", "Platform_Steam", "Steam");
}

FString FSteamExternalCredentials::GetType() const
{
    return TEXT("STEAM_SESSION_TICKET");
}

FString FSteamExternalCredentials::GetId() const
{
    return TEXT("");
}

FString FSteamExternalCredentials::GetToken() const
{
    return this->CredentialInfo.SessionTicket;
}

TMap<FString, FString> FSteamExternalCredentials::GetAuthAttributes() const
{
    return this->CredentialInfo.AuthAttributes;
}

FName FSteamExternalCredentials::GetNativeSubsystemName() const
{
    return STEAM_SUBSYSTEM;
}

void FSteamExternalCredentials::OnCredentialsRefreshed(
    const FError &ErrorCode,
    FSteamCredentialInfo NewCredentialInfo,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    if (!ErrorCode.WasSuccessful())
    {
        OnComplete.ExecuteIfBound(false);
        return;
    }

    this->CredentialInfo = MoveTemp(NewCredentialInfo);
    OnComplete.ExecuteIfBound(true);
}

void FSteamExternalCredentials::Refresh(
    TSoftObjectPtr<UWorld> InWorld,
    int32 LocalUserNum,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    FSteamCredentialObtainer::StartFromCredentialRefresh(
        InWorld,
        LocalUserNum,
        FSteamCredentialObtainer::FOnCredentialObtained::CreateSP(
            this,
            &FSteamExternalCredentials::OnCredentialsRefreshed,
            OnComplete));
}

}

#endif