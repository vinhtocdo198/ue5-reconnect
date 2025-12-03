// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

class REDPOINTEOSAPI_API FSetPresence
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Presence, SetPresence, EOS_PRESENCE_SETPRESENCE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
        ParamHelpers::TRequired<TReleasableRef<EOS_HPresenceModification>> PresenceModificationHandle;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_EpicAccountId LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()