// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Decl/SyncGet.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FGetPacketQueueInfo
{
    REDPOINT_EOSSDK_API_CALL_SYNC_GET_BEGIN(P2P, GetPacketQueueInfo, EOS_P2P_GETPACKETQUEUEINFO_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        uint64 IncomingPacketQueueMaxSizeBytes;
        uint64 IncomingPacketQueueCurrentSizeBytes;
        uint64 IncomingPacketQueueCurrentPacketCount;
        uint64 OutgoingPacketQueueMaxSizeBytes;
        uint64 OutgoingPacketQueueCurrentSizeBytes;
        uint64 OutgoingPacketQueueCurrentPacketCount;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_GET_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()