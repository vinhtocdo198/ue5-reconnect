// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FNotifySessionInviteAccepted
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Sessions,
        SessionInviteAccepted,
        EOS_SESSIONS_ADDNOTIFYSESSIONINVITEACCEPTED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        FString SessionId;
        EOS_ProductUserId LocalUserId;
        EOS_ProductUserId TargetUserId;
        FString InviteId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()