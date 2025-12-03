// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/P2P/SocketId.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FSendPacket
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(P2P, SendPacket, EOS_P2P_SENDPACKET_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<EOS_ProductUserId> RemoteUserId;
        ParamHelpers::TRequired<FSocketId> SocketId;
        ParamHelpers::TRequired<uint8> Channel;
        ParamHelpers::TRequired<uint32> DataLengthBytes;
        ParamHelpers::TRequired<const void *> Data;
        ParamHelpers::TRequired<bool> bAllowDelayedDelivery;
        ParamHelpers::TRequired<EOS_EPacketReliability> Reliability;
        ParamHelpers::TRequired<bool> bDisableAutoAcceptConnection;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()