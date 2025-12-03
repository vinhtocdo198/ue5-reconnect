// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/User/OnlineUserEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

FOnlineUserEOS::FOnlineUserEOS(const FName &InInstanceName, const TSharedRef<const FUniqueNetIdEOS> &InUserId)
    : TBaseUserImplementation(InInstanceName, InUserId)
{
}

FOnlineUserEOS::FOnlineUserEOS(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoPtr &InUserInfo)
    : TBaseUserImplementation(InInstanceName, InUserInfo)
{
}

FOnlineUserEOS::FOnlineUserEOS(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoRef &InUserInfo)
    : TBaseUserImplementation(InInstanceName, InUserInfo)
{
}

REDPOINT_EOS_CODE_GUARD_END()