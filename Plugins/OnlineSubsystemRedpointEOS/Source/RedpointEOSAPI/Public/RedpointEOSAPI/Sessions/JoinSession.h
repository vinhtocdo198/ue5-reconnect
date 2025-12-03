// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FJoinSession
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Sessions, JoinSession, EOS_SESSIONS_JOINSESSION_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SessionName;
        ParamHelpers::TRequired<TReleasableRef<EOS_HSessionDetails>> SessionHandle;
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<bool> bPresenceEnabled;
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