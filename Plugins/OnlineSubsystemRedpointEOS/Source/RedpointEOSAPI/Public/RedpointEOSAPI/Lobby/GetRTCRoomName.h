// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncGetString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FGetRTCRoomName
{
    REDPOINT_EOSSDK_API_CALL_SYNC_GET_STRING_BEGIN(Lobby, GetRTCRoomName, EOS_LOBBY_GETRTCROOMNAME_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> LobbyId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_GET_STRING_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()