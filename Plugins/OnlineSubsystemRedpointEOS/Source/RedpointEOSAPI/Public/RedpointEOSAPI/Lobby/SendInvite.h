// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FSendInvite
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Lobby, SendInvite, EOS_LOBBY_SENDINVITE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> LobbyId;
        ParamHelpers::TRequired<EOS_ProductUserId> TargetUserId;
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

REDPOINT_EOS_CODE_GUARD_END()