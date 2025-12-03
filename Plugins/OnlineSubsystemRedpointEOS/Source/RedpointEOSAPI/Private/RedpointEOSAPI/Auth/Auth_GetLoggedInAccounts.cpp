// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Auth/GetLoggedInAccounts.h"

#include "RedpointEOSAPI/Templates/Impl/SyncListNoOptions.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_IMPL(GetLoggedInAccounts)

EOS_EpicAccountId FGetLoggedInAccounts::MapGetByIndexResult(const NativeGetByIndexResult &NativeResult)
{
    return NativeResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()