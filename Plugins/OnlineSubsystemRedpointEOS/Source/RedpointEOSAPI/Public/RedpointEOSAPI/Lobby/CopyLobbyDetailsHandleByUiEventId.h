// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FCopyLobbyDetailsHandleByUiEventId
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_BEGIN(
        Lobby,
        CopyLobbyDetailsHandleByUiEventId,
        EOS_LOBBY_COPYLOBBYDETAILSHANDLEBYUIEVENTID_API_LATEST,
        LobbyDetails)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_UI_EventId> UiEventId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()