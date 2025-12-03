// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncGetStringSignedCount.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

class REDPOINTEOSAPI_API FGetJoinInfo
{
    REDPOINT_EOSSDK_API_CALL_SYNC_GET_STRING_SIGNED_COUNT_BEGIN(
        Presence,
        GetJoinInfo,
        EOS_PRESENCE_GETJOININFO_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
        ParamHelpers::TRequired<EOS_EpicAccountId> TargetUserId;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_GET_STRING_SIGNED_COUNT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()