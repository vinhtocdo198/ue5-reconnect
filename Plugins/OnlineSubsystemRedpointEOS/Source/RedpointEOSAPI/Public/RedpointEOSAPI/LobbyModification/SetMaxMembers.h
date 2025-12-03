// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyModification
{

class REDPOINTEOSAPI_API FSetMaxMembers
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        LobbyModification,
        SetMaxMembers,
        EOS_LOBBYMODIFICATION_SETMAXMEMBERS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<uint32> MaxMembers;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()