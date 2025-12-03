// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FLogin
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Connect, Login, EOS_CONNECT_LOGIN_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> Id;
        ParamHelpers::TRequired<FString> Token;
        ParamHelpers::TRequired<FString> LocalDisplayName;
        ParamHelpers::TRequired<EOS_EExternalCredentialType> Type;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
        EOS_ContinuanceToken ContinuanceToken;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()