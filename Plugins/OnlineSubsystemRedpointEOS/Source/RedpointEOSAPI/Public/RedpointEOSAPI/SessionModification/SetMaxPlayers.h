// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

class REDPOINTEOSAPI_API FSetMaxPlayers
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        SessionModification,
        SetMaxPlayers,
        EOS_SESSIONMODIFICATION_SETMAXPLAYERS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<uint32> MaxPlayers;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()