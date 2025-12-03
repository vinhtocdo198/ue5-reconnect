// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2653858584, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSUSERCACHE_API FErrorableUserInfo
{
private:
    FUserInfoPtr UserInfo;
    TSharedRef<FError> Error;

public:
    FErrorableUserInfo();
    FErrorableUserInfo(const FUserInfoRef &InUserInfo);
    FErrorableUserInfo(const FError &InError);
    FErrorableUserInfo(const FErrorableUserInfo &) = default;
    FErrorableUserInfo(FErrorableUserInfo &&) = default;
    ~FErrorableUserInfo() = default;

    bool IsErrored() const;
    FUserInfoRef GetUserInfo() const;
    TSharedRef<FError> GetError() const;
};

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(2653858584, Redpoint::EOS::UserCache, FErrorableUserInfo)
}

REDPOINT_EOS_CODE_GUARD_END()