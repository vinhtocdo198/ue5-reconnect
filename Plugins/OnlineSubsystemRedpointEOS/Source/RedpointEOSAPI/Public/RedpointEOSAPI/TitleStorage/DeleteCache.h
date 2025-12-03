// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncWithImmediateError.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::TitleStorage
{

class REDPOINTEOSAPI_API FDeleteCache
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_WITH_IMMEDIATE_ERROR_BEGIN(
        TitleStorage,
        DeleteCache,
        EOS_TITLESTORAGE_DELETECACHE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_WITH_IMMEDIATE_ERROR_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()