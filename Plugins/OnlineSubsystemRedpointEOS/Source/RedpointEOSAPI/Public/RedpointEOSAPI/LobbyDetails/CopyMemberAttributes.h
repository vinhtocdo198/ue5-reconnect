// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/LobbySessionsShared/AttributeData.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

class REDPOINTEOSAPI_API FCopyMemberAttributes
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_BEGIN(
        LobbyDetails,
        GetMemberAttributeCount,
        EOS_LOBBYDETAILS_GETMEMBERATTRIBUTECOUNT_API_LATEST,
        CopyMemberAttributeByIndex,
        EOS_LOBBYDETAILS_COPYMEMBERATTRIBUTEBYINDEX_API_LATEST,
        Lobby,
        Attribute)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> TargetUserId;
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