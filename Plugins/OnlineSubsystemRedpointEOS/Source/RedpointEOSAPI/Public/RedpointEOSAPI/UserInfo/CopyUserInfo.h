// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::UserInfo
{

class REDPOINTEOSAPI_API FCopyUserInfo
{
    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_BEGIN(UserInfo, CopyUserInfo, EOS_USERINFO_COPYUSERINFO_API_LATEST, EOS_UserInfo)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
        ParamHelpers::TRequired<EOS_EpicAccountId> TargetUserId;
    };

    class Result
    {
    public:
        FString Country;
        FString DisplayName;
        FString PreferredLanguage;
        FString Nickname;
        FString DisplayNameSanitized;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()