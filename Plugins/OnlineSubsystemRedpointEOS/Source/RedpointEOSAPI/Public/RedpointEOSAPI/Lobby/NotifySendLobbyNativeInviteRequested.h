// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FNotifySendLobbyNativeInviteRequested
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Lobby,
        SendLobbyNativeInviteRequested,
        EOS_LOBBY_ADDNOTIFYSENDLOBBYNATIVEINVITEREQUESTED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_UI_EventId UiEventId;
        EOS_ProductUserId LocalUserId;
        FString TargetNativeAccountType;
        FString TargetUserNativeAccountId;
        FString LobbyId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()