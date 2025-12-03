// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FNotifySessionInviteReceived
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Sessions,
        SessionInviteReceived,
        EOS_SESSIONS_ADDNOTIFYSESSIONINVITERECEIVED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_ProductUserId LocalUserId;
        EOS_ProductUserId TargetUserId;
        FString InviteId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()