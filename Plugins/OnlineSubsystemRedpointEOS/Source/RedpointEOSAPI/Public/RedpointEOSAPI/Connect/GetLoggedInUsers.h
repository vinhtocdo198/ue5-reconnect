// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncListNoOptions.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FGetLoggedInUsers
{
    REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_BEGIN(Connect, GetLoggedInUsersCount, GetLoggedInUserByIndex)

    typedef EOS_ProductUserId ResultEntry;

    REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()