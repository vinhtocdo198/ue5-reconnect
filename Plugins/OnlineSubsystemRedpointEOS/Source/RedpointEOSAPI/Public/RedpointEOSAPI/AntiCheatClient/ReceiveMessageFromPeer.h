// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

class REDPOINTEOSAPI_API FReceiveMessageFromPeer
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(
        AntiCheatClient,
        ReceiveMessageFromPeer,
        EOS_ANTICHEATCLIENT_RECEIVEMESSAGEFROMPEER_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_AntiCheatCommon_ClientHandle> PeerHandle;
        ParamHelpers::TRequired<uint32_t> DataLengthBytes;
        ParamHelpers::TRequired<const void *> Data;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()