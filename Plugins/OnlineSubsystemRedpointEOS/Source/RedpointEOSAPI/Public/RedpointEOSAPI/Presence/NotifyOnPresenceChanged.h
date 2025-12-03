// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

class REDPOINTEOSAPI_API FNotifyOnPresenceChanged
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Presence,
        OnPresenceChanged,
        EOS_PRESENCE_ADDNOTIFYONPRESENCECHANGED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_EpicAccountId LocalUserId;
        EOS_EpicAccountId PresenceUserId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()