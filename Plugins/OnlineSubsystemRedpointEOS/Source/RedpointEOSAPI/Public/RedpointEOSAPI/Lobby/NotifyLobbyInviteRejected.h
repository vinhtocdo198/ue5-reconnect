// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FNotifyLobbyInviteRejected
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(Lobby, LobbyInviteRejected, EOS_LOBBY_ADDNOTIFYLOBBYINVITEREJECTED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        FString InviteId;
        EOS_ProductUserId LocalUserId;
        EOS_ProductUserId TargetUserId;
        FString LobbyId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()