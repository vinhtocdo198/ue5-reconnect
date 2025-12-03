// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PresenceModification
{

class REDPOINTEOSAPI_API FSetData
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        PresenceModification,
        SetData,
        EOS_PRESENCEMODIFICATION_SETDATA_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<TMap<FString, FString>> Records;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()