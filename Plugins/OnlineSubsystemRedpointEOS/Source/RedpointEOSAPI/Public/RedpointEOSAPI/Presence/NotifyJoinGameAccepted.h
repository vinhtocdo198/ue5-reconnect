// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

class REDPOINTEOSAPI_API FNotifyJoinGameAccepted
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(Presence, JoinGameAccepted, EOS_PRESENCE_ADDNOTIFYJOINGAMEACCEPTED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        FString JoinInfo;
        EOS_EpicAccountId LocalUserId;
        EOS_EpicAccountId TargetUserId;
        EOS_UI_EventId UiEventId;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()