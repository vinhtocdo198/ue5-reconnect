// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionSearch
{

class REDPOINTEOSAPI_API FRemoveParameter
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        SessionSearch,
        RemoveParameter,
        EOS_SESSIONSEARCH_REMOVEPARAMETER_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> AttributeKey;
        ParamHelpers::TRequired<EOS_EComparisonOp> ComparisonOp;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()