// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FUpdateLobbyModification
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_BEGIN(
        Lobby,
        UpdateLobbyModification,
        EOS_LOBBY_UPDATELOBBYMODIFICATION_API_LATEST,
        LobbyModification)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> LobbyId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()