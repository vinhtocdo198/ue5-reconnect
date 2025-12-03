// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_SERVER_CODE
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace Redpoint::EOS::API::AntiCheatServer
{

class REDPOINTEOSAPI_API FUnregisterClient
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(
        AntiCheatServer,
        UnregisterClient,
        EOS_ANTICHEATSERVER_UNREGISTERCLIENT_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_AntiCheatCommon_ClientHandle> ClientHandle;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()