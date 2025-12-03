// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/LobbySessionsShared/AttributeData.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyModification
{

class REDPOINTEOSAPI_API FAddAttribute
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        LobbyModification,
        AddAttribute,
        EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<Redpoint::EOS::API::LobbySessionsShared::FAttributeData> AttributeData;
        ParamHelpers::TRequired<EOS_ELobbyAttributeVisibility> Visibility;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()