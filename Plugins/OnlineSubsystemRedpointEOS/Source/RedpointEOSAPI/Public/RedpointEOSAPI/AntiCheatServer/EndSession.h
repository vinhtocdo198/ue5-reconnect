// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_SERVER_CODE
#include "RedpointEOSAPI/Templates/Decl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace Redpoint::EOS::API::AntiCheatServer
{

class REDPOINTEOSAPI_API FEndSession
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(AntiCheatServer, EndSession, EOS_ANTICHEATSERVER_ENDSESSION_API_LATEST)

    class Options
    {
    public:
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()