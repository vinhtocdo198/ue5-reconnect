// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FUnregisterPlayers
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Sessions, UnregisterPlayers, EOS_SESSIONS_UNREGISTERPLAYERS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SessionName;
        ParamHelpers::TRequired<TArray<EOS_ProductUserId>> PlayersToUnregister;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        TArray<EOS_ProductUserId> UnregisteredPlayers;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()