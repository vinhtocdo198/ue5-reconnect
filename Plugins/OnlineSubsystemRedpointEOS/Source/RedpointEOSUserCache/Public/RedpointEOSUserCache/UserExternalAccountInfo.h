// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2034081122, Redpoint::EOS::UserCache)
{

class REDPOINTEOSUSERCACHE_API FUserExternalAccountInfo
{
public:
    FString DisplayName;
    FString AccountId;
    EOS_EExternalAccountType AccountIdType;
    TOptional<int64> LastLoginTime;
};

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(2034081122, Redpoint::EOS::UserCache, FUserExternalAccountInfo)
}

REDPOINT_EOS_CODE_GUARD_END()