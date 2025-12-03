// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/Auth/OnlineSubsystemExternalCredentials.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSAuth/ExternalCredentialType.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(270551599, Redpoint::EOS::Platform::Integration::OSSv1::Auth)
{
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;

FOnlineSubsystemExternalCredentials::FOnlineSubsystemExternalCredentials(
    FName InSubsystemName,
    EOS_EExternalCredentialType InCredentialType,
    const FOnlineProvidedCredentials &InitialCredentials,
    const FString &InAuthenticatedWithValue,
    const FString &InTokenAuthAttributeName)
    : SubsystemName(InSubsystemName)
    , CredentialType(InCredentialType)
    , CurrentCredentials(InitialCredentials)
    , AuthenticatedWithValue(InAuthenticatedWithValue)
    , TokenAuthAttributeName(InTokenAuthAttributeName)
{
}

FText FOnlineSubsystemExternalCredentials::GetProviderDisplayName() const
{
    return FText::FromString(TEXT("TODO"));
}

FString FOnlineSubsystemExternalCredentials::GetType() const
{
    return this->CurrentCredentials.Type;
}

FString FOnlineSubsystemExternalCredentials::GetId() const
{
    return this->CurrentCredentials.Id;
}

FString FOnlineSubsystemExternalCredentials::GetToken() const
{
    return this->CurrentCredentials.Token;
}

TMap<FString, FString> FOnlineSubsystemExternalCredentials::GetAuthAttributes() const
{
    TMap<FString, FString> UserAuthAttributes;
    UserAuthAttributes.Add(EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH, this->AuthenticatedWithValue);
    UserAuthAttributes.Add(this->TokenAuthAttributeName, this->CurrentCredentials.Token);
    return UserAuthAttributes;
}

FName FOnlineSubsystemExternalCredentials::GetNativeSubsystemName() const
{
    return this->SubsystemName;
}

void FOnlineSubsystemExternalCredentials::Refresh(
    TSoftObjectPtr<UWorld> InWorld,
    int32 LocalUserNum,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    if (!InWorld.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Could not refresh credentials with online subsystem, because the UWorld* pointer was null."));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    UWorld *World = InWorld.Get();

    IOnlineSubsystem *OSSSubsystem = Online::GetSubsystem(World, this->SubsystemName);
    if (OSSSubsystem == nullptr)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Could not refresh credentials with online subsystem, because the subsystem was not available."));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> OSSIdentity =
        StaticCastSharedPtr<IOnlineIdentity>(OSSSubsystem->GetIdentityInterface());
    if (!OSSIdentity.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Could not refresh credentials with online subsystem, because the identity interface was not "
                 "available."));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    this->CurrentCredentials.Type = FExternalCredentialType::ToString(this->CredentialType);
    this->CurrentCredentials.Token = OSSIdentity->GetAuthToken(LocalUserNum);
    this->CurrentCredentials.Id = OSSIdentity->GetPlayerNickname(LocalUserNum);
    if (this->CurrentCredentials.Id.IsEmpty())
    {
        this->CurrentCredentials.Id = TEXT("Anonymous");
    }

    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        Verbose,
        TEXT("Authenticating with OSS auth token: %s"),
        *this->CurrentCredentials.Token);

    OnComplete.ExecuteIfBound(true);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()