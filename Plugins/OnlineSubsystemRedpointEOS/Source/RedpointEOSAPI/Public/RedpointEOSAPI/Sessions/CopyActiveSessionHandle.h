// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FCopyActiveSessionHandle
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_BEGIN(
        Sessions,
        CopyActiveSessionHandle,
        EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST,
        ActiveSession)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SessionName;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()