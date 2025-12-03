// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FNotifyLeaveLobbyRequested
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(Lobby, LeaveLobbyRequested, EOS_LOBBY_ADDNOTIFYLEAVELOBBYREQUESTED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_ProductUserId LocalUserId;
        FString LobbyId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()