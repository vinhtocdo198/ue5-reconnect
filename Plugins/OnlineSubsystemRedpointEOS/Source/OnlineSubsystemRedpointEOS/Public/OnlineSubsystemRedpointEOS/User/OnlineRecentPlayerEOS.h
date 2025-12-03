// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/User/BaseUserImplementation.h"
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineRecentPlayerEOS
    : public TBaseUserImplementation<FOnlineRecentPlayerEOS, FOnlineRecentPlayer>,
      public TSharedFromThis<FOnlineRecentPlayerEOS>
{
    friend TBaseUserImplementation<FOnlineRecentPlayerEOS, FOnlineRecentPlayer>;

protected:
    FOnlineRecentPlayerEOS(const FName &InInstanceName, const TSharedRef<const FUniqueNetIdEOS> &InUserId);
    FOnlineRecentPlayerEOS(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoPtr &InUserInfo);

public:
    FOnlineRecentPlayerEOS(const FName &InInstanceName, const Redpoint::EOS::UserCache::FUserInfoRef &InUserInfo);
    UE_NONCOPYABLE(FOnlineRecentPlayerEOS);
    virtual ~FOnlineRecentPlayerEOS() override = default;

    FDateTime LastSeen;
    virtual FDateTime GetLastSeen() const override;
};

REDPOINT_EOS_CODE_GUARD_END()