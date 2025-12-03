// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FNotifyLobbyMemberStatusReceived
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Lobby,
        LobbyMemberStatusReceived,
        EOS_LOBBY_ADDNOTIFYLOBBYMEMBERSTATUSRECEIVED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        FString LobbyId;
        EOS_ProductUserId TargetUserId;
        EOS_ELobbyMemberStatus CurrentStatus;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()