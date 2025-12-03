// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::UserInfo
{

class REDPOINTEOSAPI_API FQueryUserInfoByDisplayName
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(
        UserInfo,
        QueryUserInfoByDisplayName,
        EOS_USERINFO_QUERYUSERINFOBYDISPLAYNAME_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
        ParamHelpers::TRequired<FString> DisplayName;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_EpicAccountId LocalUserId;
        EOS_EpicAccountId TargetUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()