// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FQueryNATType
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(P2P, QueryNATType, EOS_P2P_QUERYNATTYPE_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ENATType NATType;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()