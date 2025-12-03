// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FNotifyLobbyMemberUpdateReceived
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Lobby,
        LobbyMemberUpdateReceived,
        EOS_LOBBY_ADDNOTIFYLOBBYMEMBERUPDATERECEIVED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        FString LobbyId;
        EOS_ProductUserId TargetUserId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()