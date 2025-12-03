// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FNotifyJoinLobbyAccepted
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(Lobby, JoinLobbyAccepted, EOS_LOBBY_ADDNOTIFYJOINLOBBYACCEPTED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_ProductUserId LocalUserId;
        EOS_UI_EventId UiEventId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()