// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FUpdateLobby
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Lobby, UpdateLobby, EOS_LOBBY_UPDATELOBBY_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<TReleasableRef<EOS_HLobbyModification>> LobbyModificationHandle;
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