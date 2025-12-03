// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FLinkAccount
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Connect, LinkAccount, EOS_CONNECT_LINKACCOUNT_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ContinuanceToken> ContinuanceToken;
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
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