// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/User/BaseUserImplementation.h"
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineUserEOS : public TBaseUserImplementation<FOnlineUserEOS, FOnlineUser>,
                                                      public TSharedFromThis<FOnlineUserEOS>
{
    friend TBaseUserImplementation<FOnlineUserEOS, FOnlineUser>;

protected:
    FOnlineUserEOS(const FName &InInstanceName, const TSharedRef<const FUniqueNetIdEOS> &InUserId);
    FOnlineUserEOS(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoPtr &InUserInfo);

public:
    FOnlineUserEOS(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoRef &InUserInfo);
    UE_NONCOPYABLE(FOnlineUserEOS);
    virtual ~FOnlineUserEOS() override = default;
};

REDPOINT_EOS_CODE_GUARD_END()