// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Friends
{

class REDPOINTEOSAPI_API FNotifyBlockedUsersUpdate
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        Friends,
        BlockedUsersUpdate,
        EOS_FRIENDS_ADDNOTIFYBLOCKEDUSERSUPDATE_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
        EOS_EpicAccountId LocalUserId;
        EOS_EpicAccountId TargetUserId;
        bool bBlocked;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()