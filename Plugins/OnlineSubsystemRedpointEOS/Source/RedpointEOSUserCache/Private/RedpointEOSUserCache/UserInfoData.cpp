// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSUserCache/UserInfoData.h"

#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4215096633, Redpoint::EOS::UserCache)
{

void FUserInfoData::MaterializeAttributes(const UE::Online::FAccountId &InUserId)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API::Common;

    TMap<FString, FString> NewAttributes;
    NewAttributes.Add(TEXT("id"), GetUserIdString(InUserId));
    NewAttributes.Add(TEXT("ready"), TEXT("true"));
    NewAttributes.Add(TEXT("productUserId"), GetUserIdString(InUserId));
    if (this->MostRecentlySignedInPlatform.IsSet() &&
        this->LinkedExternalAccounts.Contains(this->MostRecentlySignedInPlatform.GetValue()))
    {
        auto DisplayName = this->LinkedExternalAccounts[this->MostRecentlySignedInPlatform.GetValue()].DisplayName;
        NewAttributes.Add(TEXT("displayName"), DisplayName);
        NewAttributes.Add(TEXT("prefDisplayName"), DisplayName);
    }
    {
        TArray<FString> ExternalAccountTypes;
        for (const auto &KV : this->LinkedExternalAccounts)
        {
            ExternalAccountTypes.Add(ExternalAccountTypeMap.Get(KV.Key).ToLower());
        }
        NewAttributes.Add(TEXT("externalAccountTypes"), FString::Join(ExternalAccountTypes, TEXT(",")));
    }
    {
        for (const auto &KV : this->LinkedExternalAccounts)
        {
            FString ExternalAccountType = ExternalAccountTypeMap.Get(KV.Key).ToLower();
            NewAttributes.Add(FString::Printf(TEXT("externalAccount.%s.id"), *ExternalAccountType), KV.Value.AccountId);

            // @note: Prefer the Epic level display name over the EOS Connect value.
            FString DisplayName =
                KV.Key == EOS_EExternalAccountType::EOS_EAT_EPIC && this->LinkedEpicGamesAccount.IsSet()
                    ? this->LinkedEpicGamesAccount.GetValue().DisplayNameSanitized
                    : KV.Value.DisplayName;
            NewAttributes.Add(
                FString::Printf(TEXT("externalAccount.%s.displayName"), *ExternalAccountType),
                DisplayName);

            if (KV.Value.LastLoginTime.IsSet())
            {
                NewAttributes.Add(
                    FString::Printf(TEXT("externalAccount.%s.lastLoginTime.unixTimestampUtc"), *ExternalAccountType),
                    FString::Printf(TEXT("%lld"), KV.Value.LastLoginTime.GetValue()));
            }

            if (KV.Key == EOS_EExternalAccountType::EOS_EAT_EPIC && this->LinkedEpicGamesAccount.IsSet())
            {
                NewAttributes.Add(
                    FString::Printf(TEXT("externalAccount.%s.country"), *ExternalAccountType),
                    this->LinkedEpicGamesAccount.GetValue().Country);
                NewAttributes.Add(
                    FString::Printf(TEXT("externalAccount.%s.nickname"), *ExternalAccountType),
                    this->LinkedEpicGamesAccount.GetValue().Nickname);
                NewAttributes.Add(
                    FString::Printf(TEXT("externalAccount.%s.preferredLanguage"), *ExternalAccountType),
                    this->LinkedEpicGamesAccount.GetValue().PreferredLanguage);
            }
        }
    }

    this->Attributes = NewAttributes;
}

}

REDPOINT_EOS_CODE_GUARD_END()