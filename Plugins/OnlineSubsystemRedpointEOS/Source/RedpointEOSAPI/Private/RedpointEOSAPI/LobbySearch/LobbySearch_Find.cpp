// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbySearch/Find.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectAsyncListReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbySearch
{

REDPOINT_EOSSDK_API_CALL_OBJECT_ASYNC_LIST_RELEASABLE_IMPL(Find);

void FFind::MapQueryOptions(NativeQueryOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FFind::MapCountOptions(NativeCountOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FFind::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.LobbyIndex = Index;
}

}

REDPOINT_EOS_CODE_GUARD_END()