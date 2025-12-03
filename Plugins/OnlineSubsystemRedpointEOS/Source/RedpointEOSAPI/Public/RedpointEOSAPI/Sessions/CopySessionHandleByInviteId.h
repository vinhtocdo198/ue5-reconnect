// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FCopySessionHandleByInviteId
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_BEGIN(
        Sessions,
        CopySessionHandleByInviteId,
        EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST,
        SessionDetails)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> InviteId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()