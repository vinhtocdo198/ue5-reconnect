// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationGOG/GOGExternalCredentials.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED

#include "RedpointEOSPlatformIntegrationGOG/GOGEncryptedAppTicketTask.h"
#include "RedpointEOSPlatformIntegrationGOG/Logging.h"
#include "RedpointEOSPlatformIntegrationGOG/SDK.h"

namespace Redpoint::EOS::Platform::Integration::GOG
{

FGOGExternalCredentials::FGOGExternalCredentials(
    const FString &InAppTicket,
    const TMap<FString, FString> &InAuthAttributes)
{
    this->SessionTicket = InAppTicket;
    this->AuthAttributes = InAuthAttributes;
}

FText FGOGExternalCredentials::GetProviderDisplayName() const
{
    return NSLOCTEXT("OnlineSubsystemRedpointEOS", "Platform_GOG", "GOG");
}

FString FGOGExternalCredentials::GetType() const
{
    return TEXT("GOG_SESSION_TICKET");
}

FString FGOGExternalCredentials::GetId() const
{
    return TEXT("");
}

FString FGOGExternalCredentials::GetToken() const
{
    return this->SessionTicket;
}

TMap<FString, FString> FGOGExternalCredentials::GetAuthAttributes() const
{
    return this->AuthAttributes;
}

FName FGOGExternalCredentials::GetNativeSubsystemName() const
{
    return FName(TEXT("GOG"));
}

void FGOGExternalCredentials::Refresh(
    TSoftObjectPtr<UWorld> InWorld,
    int32 LocalUserNum,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    TSharedPtr<FGOGEncryptedAppTicketTask> Task = MakeShared<FGOGEncryptedAppTicketTask>(
        FOnGOGAppTicketResult::CreateSP(this, &FGOGExternalCredentials::OnEncryptedAppTicketResult, OnComplete));
    Task->Start();
}

void FGOGExternalCredentials::OnEncryptedAppTicketResult(
    bool bWasSuccessful,
    FString AppTicket,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogRedpointEOSPlatformIntegrationGOG, Error, TEXT("Unable to refresh encrypted app ticket from GOG"));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    auto UserId = galaxy::api::User()->GetGalaxyID();

    this->AuthAttributes.Add(TEXT("gog.id"), FString::Printf(TEXT("%llu"), UserId.ToUint64()));
    this->AuthAttributes.Add(TEXT("gog.encryptedAppTicket"), AppTicket);

    UE_LOG(LogRedpointEOSPlatformIntegrationGOG, Verbose, TEXT("Refreshed with new GOG app ticket: %s"), *AppTicket);

    this->SessionTicket = AppTicket;
    OnComplete.ExecuteIfBound(true);
}

}

#endif