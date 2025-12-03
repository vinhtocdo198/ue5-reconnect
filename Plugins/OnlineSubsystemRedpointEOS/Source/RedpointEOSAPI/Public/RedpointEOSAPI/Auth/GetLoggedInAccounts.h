// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncListNoOptions.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

class REDPOINTEOSAPI_API FGetLoggedInAccounts
{
    REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_BEGIN(Auth, GetLoggedInAccountsCount, GetLoggedInAccountByIndex)

    typedef EOS_EpicAccountId ResultEntry;

    REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()