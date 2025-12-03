// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyModification
{

class REDPOINTEOSAPI_API FRemoveMemberAttribute
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        LobbyModification,
        RemoveMemberAttribute,
        EOS_LOBBYMODIFICATION_REMOVEMEMBERATTRIBUTE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> AttributeKey;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()