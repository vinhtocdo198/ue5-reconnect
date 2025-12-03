// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

class REDPOINTEOSAPI_API FSetAllowedPlatformIds
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        SessionModification,
        SetAllowedPlatformIds,
        EOS_SESSIONMODIFICATION_SETALLOWEDPLATFORMIDS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<TArray<Common::EOnlinePlatformType>> AllowedPlatformIds;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()