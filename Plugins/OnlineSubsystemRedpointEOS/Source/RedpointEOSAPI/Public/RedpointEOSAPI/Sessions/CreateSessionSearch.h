// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FCreateSessionSearch
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_BEGIN(
        Sessions,
        CreateSessionSearch,
        EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST,
        SessionSearch)

    class Options
    {
    public:
        ParamHelpers::TRequired<uint32> MaxSearchResults;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()