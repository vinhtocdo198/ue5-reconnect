// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FUpdateSession
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Sessions, UpdateSession, EOS_SESSIONS_UPDATESESSION_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<TReleasableRef<EOS_HSessionModification>> SessionModificationHandle;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        FString SessionName;
        FString SessionId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()