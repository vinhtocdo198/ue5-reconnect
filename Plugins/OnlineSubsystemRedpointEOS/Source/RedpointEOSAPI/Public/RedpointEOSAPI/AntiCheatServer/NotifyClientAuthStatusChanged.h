// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_SERVER_CODE
#include "RedpointEOSAPI/Templates/Decl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace Redpoint::EOS::API::AntiCheatServer
{

class REDPOINTEOSAPI_API FNotifyClientAuthStatusChanged
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        AntiCheatServer,
        ClientAuthStatusChanged,
        EOS_ANTICHEATSERVER_ADDNOTIFYCLIENTAUTHSTATUSCHANGED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_AntiCheatCommon_ClientHandle ClientHandle;
        EOS_EAntiCheatCommonClientAuthStatus ClientAuthStatus;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()