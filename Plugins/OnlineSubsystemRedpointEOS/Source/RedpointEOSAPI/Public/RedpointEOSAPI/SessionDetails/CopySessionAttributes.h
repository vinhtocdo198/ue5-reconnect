// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/LobbySessionsShared/AttributeData.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionDetails
{

class REDPOINTEOSAPI_API FCopySessionAttributes
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_BEGIN(
        SessionDetails,
        GetSessionAttributeCount,
        EOS_SESSIONDETAILS_GETSESSIONATTRIBUTECOUNT_API_LATEST,
        CopySessionAttributeByIndex,
        EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST,
        SessionDetails,
        Attribute)

    class Options
    {
    };

    class ResultEntry
    {
    public:
        LobbySessionsShared::FAttributeData Data;
        EOS_ESessionAttributeAdvertisementType AdvertisementType;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()