// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

class REDPOINTEOSAPI_API FDeletePersistentAuth
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Auth, DeletePersistentAuth, EOS_AUTH_DELETEPERSISTENTAUTH_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> RefreshToken;
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