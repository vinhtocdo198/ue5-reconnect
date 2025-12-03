// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FDumpSessionState
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(Sessions, DumpSessionState, EOS_SESSIONS_DUMPSESSIONSTATE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SessionName;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()