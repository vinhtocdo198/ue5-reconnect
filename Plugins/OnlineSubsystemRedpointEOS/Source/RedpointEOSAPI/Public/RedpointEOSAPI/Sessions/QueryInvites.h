// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncListGetString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FQueryInvites
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_GET_STRING_BEGIN(
        Sessions,
        QueryInvites,
        EOS_SESSIONS_QUERYINVITES_API_LATEST,
        GetInviteCount,
        EOS_SESSIONS_GETINVITECOUNT_API_LATEST,
        GetInviteIdByIndex,
        EOS_SESSIONS_GETINVITEIDBYINDEX_API_LATEST,
        EOS_SESSIONS_INVITEID_MAX_LENGTH)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_GET_STRING_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()