// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FDestroySession
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Sessions, DestroySession, EOS_SESSIONS_DESTROYSESSION_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SessionName;
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