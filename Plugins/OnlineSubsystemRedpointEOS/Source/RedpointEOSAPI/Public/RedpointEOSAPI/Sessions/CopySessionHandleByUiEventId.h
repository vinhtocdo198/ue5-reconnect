// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FCopySessionHandleByUiEventId
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_BEGIN(
        Sessions,
        CopySessionHandleByUiEventId,
        EOS_SESSIONS_COPYSESSIONHANDLEBYUIEVENTID_API_LATEST,
        SessionDetails)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_UI_EventId> UiEventId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()