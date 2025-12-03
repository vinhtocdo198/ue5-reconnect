// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Friends/GetBlockedUsers.h"

#include "RedpointEOSAPI/Templates/Impl/SyncList.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Friends
{

REDPOINT_EOSSDK_API_CALL_SYNC_LIST_IMPL(GetBlockedUsers)

void FGetBlockedUsers::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FGetBlockedUsers::MapGetByIndexOptions(
    NativeGetByIndexOptions &NativeOptions,
    const Options &Options,
    NativeCount Index,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.Index = Index;
}

EOS_EpicAccountId FGetBlockedUsers::MapGetByIndexResult(const NativeGetByIndexResult &NativeResult)
{
    return NativeResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()