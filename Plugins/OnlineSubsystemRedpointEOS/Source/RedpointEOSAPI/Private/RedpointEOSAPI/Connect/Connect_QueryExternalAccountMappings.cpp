// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/QueryExternalAccountMappings.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(QueryExternalAccountMappings);

void FQueryExternalAccountMappings::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.AccountIdType = *Options.AccountIdType;

    TArray<const char *> ExternalAccountIdsConstChar;
    for (const auto &Entry : *Options.ExternalAccountIds)
    {
        ExternalAccountIdsConstChar.Add(Allocator.AsUtf8(Entry, Templates::Util::EAllocationFlags::Default));
    }
    NativeOptions.ExternalAccountIds = const_cast<const char **>(Allocator.AllocateArray(
        ExternalAccountIdsConstChar,
        NativeOptions.ExternalAccountIdCount,
        Templates::Util::EAllocationFlags::Default));
}

void FQueryExternalAccountMappings::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()