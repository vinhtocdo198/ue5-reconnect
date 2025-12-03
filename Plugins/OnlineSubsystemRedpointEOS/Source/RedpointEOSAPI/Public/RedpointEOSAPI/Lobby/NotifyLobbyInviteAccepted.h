// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FNotifyLobbyInviteAccepted
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(Lobby, LobbyInviteAccepted, EOS_LOBBY_ADDNOTIFYLOBBYINVITEACCEPTED_API_LATEST)

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