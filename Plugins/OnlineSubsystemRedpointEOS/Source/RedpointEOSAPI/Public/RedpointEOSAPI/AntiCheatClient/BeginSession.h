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

class REDPOINTEOSAPI_API FBeginSession
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(AntiCheatClient, BeginSession, EOS_ANTICHEATCLIENT_BEGINSESSION_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<EOS_EAntiCheatClientMode> Mode;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()