// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FNotifyRTCRoomConnectionChanged
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Lobby,
        RTCRoomConnectionChanged,
        EOS_LOBBY_ADDNOTIFYRTCROOMCONNECTIONCHANGED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        FString LobbyId;
        EOS_ProductUserId LocalUserId;
        bool bIsConnected;
        EOS_EResult DisconnectReason;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()