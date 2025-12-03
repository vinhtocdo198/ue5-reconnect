// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Friends
{

class REDPOINTEOSAPI_API FQueryFriends
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Friends, QueryFriends, EOS_FRIENDS_QUERYFRIENDS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_EpicAccountId LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()