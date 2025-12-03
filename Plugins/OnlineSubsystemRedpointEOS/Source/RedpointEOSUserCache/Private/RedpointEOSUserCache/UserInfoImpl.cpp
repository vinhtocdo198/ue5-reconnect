// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSUserCache/UserInfoImpl.h"

#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1948841835, Redpoint::EOS::UserCache)
{

TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> FUserInfoImpl::UnreadyLinkedExternalAccounts =
    TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo>();

FUserInfoImpl::FUserInfoImpl(const UE::Online::FAccountId &InUserId)
    : UserId(InUserId)
    , UserInfoData(nullptr)
    , UnreadyAttributes()
{
    using namespace ::Redpoint::EOS::Core::Id;
    this->UnreadyAttributes.Add(TEXT("id"), GetUserIdString(InUserId));
    this->UnreadyAttributes.Add(TEXT("ready"), TEXT("false"));
}

FUserInfoImpl::FUserInfoImpl(const UE::Online::FAccountId &InUserId, const FUserInfoDataConstRef &InExistingData)
    : UserId(InUserId)
    , UserInfoData(InExistingData)
    , UnreadyAttributes()
{
    using namespace ::Redpoint::EOS::Core::Id;
    this->UnreadyAttributes.Add(TEXT("id"), GetUserIdString(InUserId));
    this->UnreadyAttributes.Add(TEXT("ready"), TEXT("false"));
}

bool FUserInfoImpl::IsReady() const
{
    return this->UserInfoData.IsValid();
}

UE::Online::FAccountId FUserInfoImpl::GetUserId() const
{
    return this->UserId;
}

TOptional<EOS_EExternalAccountType> FUserInfoImpl::GetMostRecentlySignedInPlatform() const
{
    if (!this->UserInfoData.IsValid())
    {
        return TOptional<EOS_EExternalAccountType>();
    }

    return this->UserInfoData->MostRecentlySignedInPlatform;
}

const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &FUserInfoImpl::GetLinkedExternalAccounts() const
{
    if (!this->UserInfoData.IsValid())
    {
        return FUserInfoImpl::UnreadyLinkedExternalAccounts;
    }

    return this->UserInfoData->LinkedExternalAccounts;
}

TOptional<const FUserEpicGamesAccountInfo> FUserInfoImpl::GetLinkedEpicGamesAccount() const
{
    if (!this->UserInfoData.IsValid())
    {
        return TOptional<const FUserEpicGamesAccountInfo>();
    }

    return this->UserInfoData->LinkedEpicGamesAccount;
}

const TMap<FString, FString> &FUserInfoImpl::GetAttributes() const
{
    if (!this->UserInfoData.IsValid())
    {
        return this->UnreadyAttributes;
    }

    return this->UserInfoData->Attributes;
}

void FUserInfoImpl::SetUserInfoData(const FUserInfoDataConstRef &InUserInfoData)
{
    this->UserInfoData = InUserInfoData;
}

}

REDPOINT_EOS_CODE_GUARD_END()