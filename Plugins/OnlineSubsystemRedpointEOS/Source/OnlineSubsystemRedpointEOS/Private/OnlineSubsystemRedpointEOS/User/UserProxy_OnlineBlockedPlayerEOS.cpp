// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/User/OnlineBlockedPlayerEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

FOnlineBlockedPlayerEOS::FOnlineBlockedPlayerEOS(
    const FName &InInstanceName,
    const TSharedRef<const FUniqueNetIdEOS> &InUserId)
    : TBaseUserImplementation(InInstanceName, InUserId)
{
}

FOnlineBlockedPlayerEOS::FOnlineBlockedPlayerEOS(
    const FName &InInstanceName,
    const Redpoint::EOS::UserCache::FUserInfoPtr &InUserInfo)
    : TBaseUserImplementation(InInstanceName, InUserInfo)
{
}

FOnlineBlockedPlayerEOS::FOnlineBlockedPlayerEOS(
    const FName &InInstanceName,
    const Redpoint::EOS::UserCache::FUserInfoRef &InUserInfo)
    : TBaseUserImplementation(InInstanceName, InUserInfo)
{
}

REDPOINT_EOS_CODE_GUARD_END()