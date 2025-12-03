// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/User/OnlineRecentPlayerEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

FOnlineRecentPlayerEOS::FOnlineRecentPlayerEOS(
    const FName &InInstanceName,
    const TSharedRef<const FUniqueNetIdEOS> &InUserId)
    : TBaseUserImplementation(InInstanceName, InUserId)
{
}

FOnlineRecentPlayerEOS::FOnlineRecentPlayerEOS(
    const FName &InInstanceName,
    const Redpoint::EOS::UserCache::FUserInfoPtr &InUserInfo)
    : TBaseUserImplementation(InInstanceName, InUserInfo)
{
}

FOnlineRecentPlayerEOS::FOnlineRecentPlayerEOS(
    const FName &InInstanceName,
    const Redpoint::EOS::UserCache::FUserInfoRef &InUserInfo)
    : TBaseUserImplementation(InInstanceName, InUserInfo)
{
}

FDateTime FOnlineRecentPlayerEOS::GetLastSeen() const
{
    return this->LastSeen;
}

REDPOINT_EOS_CODE_GUARD_END()