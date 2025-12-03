// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FNotifySessionInviteRejected
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Sessions,
        SessionInviteRejected,
        EOS_SESSIONS_ADDNOTIFYSESSIONINVITEREJECTED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        FString InviteId;
        EOS_ProductUserId LocalUserId;
        EOS_ProductUserId TargetUserId;
        FString SessionId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()