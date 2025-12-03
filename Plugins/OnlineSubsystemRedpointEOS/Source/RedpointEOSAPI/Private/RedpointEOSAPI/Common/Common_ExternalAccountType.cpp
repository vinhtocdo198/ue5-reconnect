// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Common/ExternalAccountType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Common
{

FExternalAccountTypeMap ExternalAccountTypeMap = FExternalAccountTypeMap();

FExternalAccountTypeMap::FExternalAccountTypeMap()
{
    this->ToValueMap.Add(TEXT("EPIC"), EOS_EExternalAccountType::EOS_EAT_EPIC);
    this->ToValueMap.Add(TEXT("STEAM"), EOS_EExternalAccountType::EOS_EAT_STEAM);
    this->ToValueMap.Add(TEXT("PSN"), EOS_EExternalAccountType::EOS_EAT_PSN);
    this->ToValueMap.Add(TEXT("XBL"), EOS_EExternalAccountType::EOS_EAT_XBL);
    this->ToValueMap.Add(TEXT("DISCORD"), EOS_EExternalAccountType::EOS_EAT_DISCORD);
    this->ToValueMap.Add(TEXT("GOG"), EOS_EExternalAccountType::EOS_EAT_GOG);
    this->ToValueMap.Add(TEXT("NINTENDO"), EOS_EExternalAccountType::EOS_EAT_NINTENDO);
    this->ToValueMap.Add(TEXT("UPLAY"), EOS_EExternalAccountType::EOS_EAT_UPLAY);
    this->ToValueMap.Add(TEXT("OPENID"), EOS_EExternalAccountType::EOS_EAT_OPENID);
    this->ToValueMap.Add(TEXT("APPLE"), EOS_EExternalAccountType::EOS_EAT_APPLE);
    this->ToValueMap.Add(TEXT("GOOGLE"), EOS_EExternalAccountType::EOS_EAT_GOOGLE);
    this->ToValueMap.Add(TEXT("OCULUS"), EOS_EExternalAccountType::EOS_EAT_OCULUS);
    this->ToValueMap.Add(TEXT("ITCHIO"), EOS_EExternalAccountType::EOS_EAT_ITCHIO);
    this->ToValueMap.Add(TEXT("AMAZON"), EOS_EExternalAccountType::EOS_EAT_AMAZON);
#if EOS_VERSION_AT_LEAST(1, 16, 2)
    this->ToValueMap.Add(TEXT("VIVEPORT"), EOS_EExternalAccountType::EOS_EAT_VIVEPORT);
#endif
    for (const auto &KV : this->ToValueMap)
    {
        this->FromValueMap.Add(KV.Value, KV.Key);
    }
}

EOS_EExternalAccountType FExternalAccountTypeMap::Get(const FString &InValue) const
{
    auto *Value = this->ToValueMap.Find(InValue);
    checkf(
        Value != nullptr,
        TEXT("Expected %s to be a known string representation of EOS_EExternalAccountType."),
        *InValue);
    return *Value;
}

FString FExternalAccountTypeMap::Get(const EOS_EExternalAccountType &InValue) const
{
    auto *Value = this->FromValueMap.Find(InValue);
    checkf(Value != nullptr, TEXT("Expected %d to be a known value of EOS_EExternalAccountType."), (int32)InValue);
    return *Value;
}

}

REDPOINT_EOS_CODE_GUARD_END()