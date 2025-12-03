// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionSearch
{

class REDPOINTEOSAPI_API FSetMaxResults
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        SessionSearch,
        SetMaxResults,
        EOS_SESSIONSEARCH_SETMAXSEARCHRESULTS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<uint32> MaxSearchResults;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()