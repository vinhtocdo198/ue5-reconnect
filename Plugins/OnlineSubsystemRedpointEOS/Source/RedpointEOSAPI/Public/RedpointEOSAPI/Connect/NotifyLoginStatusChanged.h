// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FNotifyLoginStatusChanged
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Connect,
        LoginStatusChanged,
        EOS_CONNECT_ADDNOTIFYLOGINSTATUSCHANGED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_ProductUserId LocalUserId;
        EOS_ELoginStatus PreviousStatus;
        EOS_ELoginStatus CurrentStatus;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()