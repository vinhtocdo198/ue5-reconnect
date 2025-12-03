// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncGetTwo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FGetPortRange
{
    REDPOINT_EOSSDK_API_CALL_SYNC_GET_TWO_BEGIN(P2P, GetPortRange, EOS_P2P_GETPORTRANGE_API_LATEST)

    class Options
    {
    public:
    };

    using Result1 = uint16_t;
    using Result2 = uint16_t;

    REDPOINT_EOSSDK_API_CALL_SYNC_GET_TWO_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()