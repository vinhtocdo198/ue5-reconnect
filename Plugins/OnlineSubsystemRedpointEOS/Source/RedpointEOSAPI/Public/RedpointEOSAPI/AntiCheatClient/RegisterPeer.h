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

class REDPOINTEOSAPI_API FRegisterPeer
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(AntiCheatClient, RegisterPeer, EOS_ANTICHEATCLIENT_REGISTERPEER_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_AntiCheatCommon_ClientHandle> PeerHandle;
        ParamHelpers::TRequired<EOS_EAntiCheatCommonClientType> ClientType;
        ParamHelpers::TRequired<EOS_EAntiCheatCommonClientPlatform> ClientPlatform;
        ParamHelpers::TRequired<uint32_t> AuthenticationTimeout;
        ParamHelpers::TRequired<FString> IpAddress;
        ParamHelpers::TRequired<EOS_ProductUserId> PeerProductUserId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()