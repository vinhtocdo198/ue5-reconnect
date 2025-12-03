// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FNotifyIncomingPacketQueueFull
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        P2P,
        IncomingPacketQueueFull,
        EOS_P2P_ADDNOTIFYINCOMINGPACKETQUEUEFULL_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        uint64 PacketQueueMaxSizeBytes;
        uint64 PacketQueueCurrentSizeBytes;
        EOS_ProductUserId OverflowPacketLocalUserId;
        uint8 OverflowPacketChannel;
        uint32 OverflowPacketSizeBytes;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()