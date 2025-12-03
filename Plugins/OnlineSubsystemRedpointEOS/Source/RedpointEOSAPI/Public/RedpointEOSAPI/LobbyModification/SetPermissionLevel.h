// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyModification
{

class REDPOINTEOSAPI_API FSetPermissionLevel
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        LobbyModification,
        SetPermissionLevel,
        EOS_LOBBYMODIFICATION_SETPERMISSIONLEVEL_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ELobbyPermissionLevel> PermissionLevel;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()