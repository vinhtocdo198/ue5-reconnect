// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FNotifySendSessionNativeInviteRequested
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Sessions,
        SendSessionNativeInviteRequested,
        EOS_SESSIONS_ADDNOTIFYSENDSESSIONNATIVEINVITEREQUESTED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_UI_EventId UiEventId;
        EOS_ProductUserId LocalUserId;
        EOS_IntegratedPlatformType TargetNativeAccountType;
        FString TargetUserNativeAccountId;
        FString SessionId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()