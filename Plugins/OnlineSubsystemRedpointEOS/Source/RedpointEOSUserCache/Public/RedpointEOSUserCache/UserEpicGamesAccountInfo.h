// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3209482916, Redpoint::EOS::UserCache)
{

class REDPOINTEOSUSERCACHE_API FUserEpicGamesAccountInfo
{
public:
    EOS_EpicAccountId AccountId;
    FString Country;
    FString DisplayName;
    FString PreferredLanguage;
    FString Nickname;
    FString DisplayNameSanitized;
};

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(3209482916, Redpoint::EOS::UserCache, FUserEpicGamesAccountInfo)
}

REDPOINT_EOS_CODE_GUARD_END()