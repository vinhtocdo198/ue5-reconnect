// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FIsUserInSession
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(Sessions, IsUserInSession, EOS_SESSIONS_ISUSERINSESSION_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SessionName;
        ParamHelpers::TRequired<EOS_ProductUserId> TargetUserId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()