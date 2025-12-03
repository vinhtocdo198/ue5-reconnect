// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/ActiveSession/GetRegisteredPlayers.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSyncList.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::ActiveSession
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_IMPL(GetRegisteredPlayers);

void FGetRegisteredPlayers::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FGetRegisteredPlayers::MapGetByIndexOptions(
    NativeGetByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.PlayerIndex = Index;
}

void FGetRegisteredPlayers::MapGetByIndexResult(
    ResultEntry &Result,
    const NativeGetByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result = NativeResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()