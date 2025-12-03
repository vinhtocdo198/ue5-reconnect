// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FHardMuteMember
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Lobby, HardMuteMember, EOS_LOBBY_HARDMUTEMEMBER_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> LobbyId;
        ParamHelpers::TRequired<EOS_ProductUserId> TargetUserId;
        ParamHelpers::TRequired<bool> bHardMute;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        FString LobbyId;
        EOS_ProductUserId TargetUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()