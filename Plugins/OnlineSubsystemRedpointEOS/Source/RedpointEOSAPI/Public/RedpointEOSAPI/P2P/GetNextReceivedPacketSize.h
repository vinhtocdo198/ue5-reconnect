// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncGet.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FGetNextReceivedPacketSize
{
    REDPOINT_EOSSDK_API_CALL_SYNC_GET_BEGIN(
        P2P,
        GetNextReceivedPacketSize,
        EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<TOptional<uint8>> RequestedChannel;
    };

    using Result = uint32_t;

    REDPOINT_EOSSDK_API_CALL_SYNC_GET_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()