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

class REDPOINTEOSAPI_API FBeginSession
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(AntiCheatServer, BeginSession, EOS_ANTICHEATSERVER_BEGINSESSION_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<uint32_t> RegisterTimeoutSeconds;
        ParamHelpers::TRequired<FString> ServerName;
        ParamHelpers::TRequired<bool> bEnableGameplayData;
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()