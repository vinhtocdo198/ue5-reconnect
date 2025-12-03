// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/LobbySessionsShared/AttributeData.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

class REDPOINTEOSAPI_API FAddAttribute
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        SessionModification,
        AddAttribute,
        EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<Redpoint::EOS::API::LobbySessionsShared::FAttributeData> AttributeData;
        ParamHelpers::TRequired<EOS_ESessionAttributeAdvertisementType> AdvertisementType;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()