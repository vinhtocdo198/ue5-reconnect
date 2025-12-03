// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/UserOnlineAccountEOSImpl.h"

#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1969834351, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity)
{

FUserOnlineAccountEOSImpl::FUserOnlineAccountEOSImpl(const FIdentityUserRef &InIdentityUser)
    : IdentityUser(InIdentityUser)
{
}

FUniqueNetIdRef FUserOnlineAccountEOSImpl::GetUserId() const
{
    if (Redpoint::EOS::Core::Id::IsDedicatedServerId(this->IdentityUser->GetUserId()))
    {
        return FUniqueNetIdEOS::DedicatedServerId();
    }
    else
    {
        auto UserId = Redpoint::EOS::Core::Id::GetUniqueNetId(this->IdentityUser->GetUserId());
        if (UserId.IsValid())
        {
            return UserId.ToSharedRef();
        }
        else
        {
            return FUniqueNetIdEOS::InvalidId();
        }
    }
}

FString FUserOnlineAccountEOSImpl::GetRealName() const
{
    return TEXT("");
}

FString FUserOnlineAccountEOSImpl::GetDisplayName(const FString &Platform) const
{
    auto UserInfo = this->IdentityUser->GetUserInfo();
    if (UserInfo.IsValid())
    {
        return UserInfo->GetAttributes().FindRef(TEXT("displayName"));
    }
    return TEXT("");
}

bool FUserOnlineAccountEOSImpl::GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const
{
    auto UserInfo = this->IdentityUser->GetUserInfo();
    if (UserInfo.IsValid())
    {
        auto *Value = UserInfo->GetAttributes().Find(AttrName);
        if (Value != nullptr)
        {
            OutAttrValue = *Value;
            return true;
        }
    }
    OutAttrValue = TEXT("");
    return false;
}

FString FUserOnlineAccountEOSImpl::GetAccessToken() const
{
    return this->IdentityUser->GetIdToken();
}

bool FUserOnlineAccountEOSImpl::GetAuthAttribute(const FString &AttrName, FString &OutAttrValue) const
{
    return this->IdentityUser->TryGetAuthenticationAttributeValue(AttrName, OutAttrValue);
}

bool FUserOnlineAccountEOSImpl::SetUserAttribute(const FString &AttrName, const FString &AttrValue)
{
    return false;
}

}

REDPOINT_EOS_CODE_GUARD_END()