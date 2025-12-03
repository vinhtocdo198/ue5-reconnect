// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/GetExternalAccountMapping.h"

#include "RedpointEOSAPI/Templates/Impl/SyncResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_IMPL(GetExternalAccountMapping);

void FGetExternalAccountMapping::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.AccountIdType = *Options.AccountIdType;
    NativeOptions.TargetExternalUserId = Allocator.AsUtf8(*Options.TargetExternalAccountId);
}

EOS_ProductUserId FGetExternalAccountMapping::MapResult(const NativeResult &NativeResult)
{
    return NativeResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()