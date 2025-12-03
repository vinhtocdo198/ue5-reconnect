// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FCreateUser
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Connect, CreateUser, EOS_CONNECT_CREATEUSER_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ContinuanceToken> ContinuanceToken;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()