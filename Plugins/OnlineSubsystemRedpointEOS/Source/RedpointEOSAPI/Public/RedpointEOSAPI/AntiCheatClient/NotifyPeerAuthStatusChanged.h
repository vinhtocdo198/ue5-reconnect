// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Decl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

class REDPOINTEOSAPI_API FNotifyPeerAuthStatusChanged
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        AntiCheatClient,
        PeerAuthStatusChanged,
        EOS_ANTICHEATCLIENT_ADDNOTIFYPEERAUTHSTATUSCHANGED_API_LATEST)

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