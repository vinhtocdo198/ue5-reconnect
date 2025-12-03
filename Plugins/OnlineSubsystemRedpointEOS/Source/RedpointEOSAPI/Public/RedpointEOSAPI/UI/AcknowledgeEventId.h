// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::UI
{

class REDPOINTEOSAPI_API FAcknowledgeEventId
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(UI, AcknowledgeEventId, EOS_UI_ACKNOWLEDGEEVENTID_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_UI_EventId> UiEventId;
        ParamHelpers::TRequired<EOS_EResult> Result;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()