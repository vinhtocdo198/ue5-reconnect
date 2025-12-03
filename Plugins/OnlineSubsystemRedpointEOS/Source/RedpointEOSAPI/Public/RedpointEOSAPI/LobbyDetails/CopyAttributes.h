// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/LobbySessionsShared/AttributeData.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

class REDPOINTEOSAPI_API FCopyAttributes
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_BEGIN(
        LobbyDetails,
        GetAttributeCount,
        EOS_LOBBYDETAILS_GETATTRIBUTECOUNT_API_LATEST,
        CopyAttributeByIndex,
        EOS_LOBBYDETAILS_COPYATTRIBUTEBYINDEX_API_LATEST,
        Lobby,
        Attribute)

    class Options
    {
    };

    class ResultEntry
    {
    public:
        LobbySessionsShared::FAttributeData Data;
        bool bPublic;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()