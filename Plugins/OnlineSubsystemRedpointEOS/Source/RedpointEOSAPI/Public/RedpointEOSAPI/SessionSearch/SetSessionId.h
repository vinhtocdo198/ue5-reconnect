// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionSearch
{

class REDPOINTEOSAPI_API FSetSessionId
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(SessionSearch, SetSessionId, EOS_SESSIONSEARCH_SETSESSIONID_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SessionId;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()