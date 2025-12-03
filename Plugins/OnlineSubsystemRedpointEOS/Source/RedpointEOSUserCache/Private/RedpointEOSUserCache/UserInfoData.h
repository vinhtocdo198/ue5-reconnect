// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSUserCache/UserEpicGamesAccountInfo.h"
#include "RedpointEOSUserCache/UserExternalAccountInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4215096633, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS::UserCache;

class FUserInfoData
{
public:
    TOptional<EOS_EExternalAccountType> MostRecentlySignedInPlatform;
    TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> LinkedExternalAccounts;
    TOptional<const FUserEpicGamesAccountInfo> LinkedEpicGamesAccount;
    TMap<FString, FString> Attributes;

    void MaterializeAttributes(const UE::Online::FAccountId &InUserId);
};

typedef TSharedRef<const FUserInfoData> FUserInfoDataConstRef;
typedef TSharedPtr<const FUserInfoData> FUserInfoDataConstPtr;

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(4215096633, Redpoint::EOS::UserCache, FUserInfoData)
REDPOINT_EOS_FILE_NS_EXPORT(4215096633, Redpoint::EOS::UserCache, FUserInfoDataConstRef)
REDPOINT_EOS_FILE_NS_EXPORT(4215096633, Redpoint::EOS::UserCache, FUserInfoDataConstPtr)
}

REDPOINT_EOS_CODE_GUARD_END()