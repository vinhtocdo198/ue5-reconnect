// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FVerifyIdToken
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Connect, VerifyIdToken, EOS_CONNECT_VERIFYIDTOKEN_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> ProductUserId;
        ParamHelpers::TRequired<FString> JsonWebToken;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()