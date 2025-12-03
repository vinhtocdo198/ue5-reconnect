// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/GetLoggedInUsers.h"

#include "RedpointEOSAPI/Templates/Impl/SyncListNoOptions.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_IMPL(GetLoggedInUsers)

EOS_ProductUserId FGetLoggedInUsers::MapGetByIndexResult(const NativeGetByIndexResult &NativeResult)
{
    return NativeResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()