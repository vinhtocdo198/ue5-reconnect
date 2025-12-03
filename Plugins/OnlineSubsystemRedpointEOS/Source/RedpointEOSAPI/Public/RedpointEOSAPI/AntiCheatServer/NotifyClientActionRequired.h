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

class REDPOINTEOSAPI_API FNotifyClientActionRequired
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        AntiCheatServer,
        ClientActionRequired,
        EOS_ANTICHEATSERVER_ADDNOTIFYCLIENTACTIONREQUIRED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_AntiCheatCommon_ClientHandle ClientHandle;
        EOS_EAntiCheatCommonClientAction ClientAction;
        EOS_EAntiCheatCommonClientActionReason ActionReasonCode;
        FString ActionReasonDetailsString;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()