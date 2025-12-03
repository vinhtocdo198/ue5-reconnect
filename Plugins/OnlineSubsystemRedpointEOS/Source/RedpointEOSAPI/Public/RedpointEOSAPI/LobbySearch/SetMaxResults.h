// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbySearch
{

class REDPOINTEOSAPI_API FSetMaxResults
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(LobbySearch, SetMaxResults, EOS_LOBBYSEARCH_SETMAXRESULTS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<uint32> MaxResults;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()