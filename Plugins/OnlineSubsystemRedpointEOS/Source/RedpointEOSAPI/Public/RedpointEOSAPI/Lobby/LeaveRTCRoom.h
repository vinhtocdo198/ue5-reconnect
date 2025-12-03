// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#if EOS_VERSION_AT_LEAST(1, 17, 0)
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if EOS_VERSION_AT_LEAST(1, 17, 0)

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FLeaveRTCRoom
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Lobby, LeaveRTCRoom, EOS_LOBBY_LEAVERTCROOM_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> LobbyId;
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        FString LobbyId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()