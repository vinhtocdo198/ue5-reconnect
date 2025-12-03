// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PresenceModification
{

class REDPOINTEOSAPI_API FDeleteData
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        PresenceModification,
        DeleteData,
        EOS_PRESENCEMODIFICATION_DELETEDATA_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<TSet<FString>> RecordIds;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()