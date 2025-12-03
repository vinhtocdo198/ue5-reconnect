// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FSetPacketQueueSize
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(P2P, SetPacketQueueSize, EOS_P2P_SETPACKETQUEUESIZE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<uint64> IncomingPacketQueueMaxSizeBytes;
        ParamHelpers::TRequired<uint64> OutgoingPacketQueueMaxSizeBytes;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()