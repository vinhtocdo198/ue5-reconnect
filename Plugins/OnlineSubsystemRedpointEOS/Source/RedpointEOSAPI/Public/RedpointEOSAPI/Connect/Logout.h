// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/SDK.h"
#if EOS_VERSION_AT_LEAST(1, 16, 2)
#include "RedpointEOSAPI/Templates/Decl/Async.h"
#else
#include "RedpointEOSAPI/Templates/Decl/AsyncStub.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FLogout
{
#if EOS_VERSION_AT_LEAST(1, 16, 2)
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Connect, Logout, EOS_CONNECT_LOGOUT_API_LATEST)
#else
    REDPOINT_EOSSDK_API_CALL_ASYNC_STUB_BEGIN(Connect, Logout)
#endif

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
    };

#if EOS_VERSION_AT_LEAST(1, 16, 2)
    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
#else
    REDPOINT_EOSSDK_API_CALL_ASYNC_STUB_END()
#endif
};

}

REDPOINT_EOS_CODE_GUARD_END()