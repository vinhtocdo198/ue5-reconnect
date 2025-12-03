// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Friends
{

class REDPOINTEOSAPI_API FGetStatus
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_BEGIN(Friends, GetStatus, EOS_FRIENDS_GETSTATUS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
        ParamHelpers::TRequired<EOS_EpicAccountId> TargetUserId;
    };

    typedef EOS_EFriendsStatus Result;

    REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()