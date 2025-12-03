// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/QueryInvites.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncListGetString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_GET_STRING_IMPL(QueryInvites);

void FQueryInvites::MapQueryOptions(
    NativeQueryOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FQueryInvites::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FQueryInvites::MapGetByIndexOptions(
    NativeGetByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.Index = Index;
}

}

REDPOINT_EOS_CODE_GUARD_END()