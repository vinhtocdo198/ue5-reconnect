// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/UserInfo/GetLocalPlatformType.h"

#include "RedpointEOSAPI/Templates/Impl/SyncResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::UserInfo
{

REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_IMPL(GetLocalPlatformType);

void FGetLocalPlatformType::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

Redpoint::EOS::API::Common::EOnlinePlatformType FGetLocalPlatformType::MapResult(const NativeResult &NativeResult)
{
    return (Redpoint::EOS::API::Common::EOnlinePlatformType)NativeResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()