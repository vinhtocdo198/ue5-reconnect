// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbySearch
{

class REDPOINTEOSAPI_API FSetLobbyId
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(LobbySearch, SetLobbyId, EOS_LOBBYSEARCH_SETLOBBYID_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> LobbyId;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()