// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationItchIo/Auth/ItchIoExternalCredentials.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3076326616, Redpoint::EOS::Platform::Integration::ItchIo::Auth)
{

FItchIoExternalCredentials::FItchIoExternalCredentials(
    const FString &InApiKey,
    const TMap<FString, FString> &InAuthAttributes)
    : ApiKey(InApiKey)
    , AuthAttributes(InAuthAttributes)
{
}

FText FItchIoExternalCredentials::GetProviderDisplayName() const
{
    return NSLOCTEXT("OnlineSubsystemRedpointEOS", "ItchIoProviderName", "itch.io");
}

FString FItchIoExternalCredentials::GetType() const
{
    return TEXT("ITCHIO_KEY");
}

FString FItchIoExternalCredentials::GetId() const
{
    return TEXT("");
}

FString FItchIoExternalCredentials::GetToken() const
{
    return this->ApiKey;
}

ESupportedLinkType FItchIoExternalCredentials::GetProviderSupportedLinkType(FName PlatformName) const
{
    return ESupportedLinkType::None;
}

TMap<FString, FString> FItchIoExternalCredentials::GetAuthAttributes() const
{
    return this->AuthAttributes;
}

FName FItchIoExternalCredentials::GetNativeSubsystemName() const
{
    return NAME_None;
}

void FItchIoExternalCredentials::Refresh(
    TSoftObjectPtr<UWorld> InWorld,
    int32 LocalUserNum,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    // itch.io does not support refreshing credentials.
    OnComplete.ExecuteIfBound(true);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
