// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FNotifyAuthExpiration
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(Connect, AuthExpiration, EOS_CONNECT_ADDNOTIFYAUTHEXPIRATION_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_ProductUserId LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()