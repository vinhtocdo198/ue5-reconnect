// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/User/BaseUserImplementation.h"
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineBlockedPlayerEOS
    : public TBaseUserImplementation<FOnlineBlockedPlayerEOS, FOnlineBlockedPlayer>,
      public TSharedFromThis<FOnlineBlockedPlayerEOS>
{
    friend TBaseUserImplementation<FOnlineBlockedPlayerEOS, FOnlineBlockedPlayer>;

protected:
    FOnlineBlockedPlayerEOS(const FName &InInstanceName, const TSharedRef<const FUniqueNetIdEOS> &InUserId);
    FOnlineBlockedPlayerEOS(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoPtr &InUserInfo);

public:
    FOnlineBlockedPlayerEOS(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoRef &InUserInfo);
    UE_NONCOPYABLE(FOnlineBlockedPlayerEOS);
    virtual ~FOnlineBlockedPlayerEOS() override = default;
};

REDPOINT_EOS_CODE_GUARD_END()