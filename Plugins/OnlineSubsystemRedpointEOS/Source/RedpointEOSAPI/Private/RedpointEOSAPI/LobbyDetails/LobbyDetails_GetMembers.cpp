// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyDetails/GetMembers.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSyncList.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_IMPL(GetMembers);

void FGetMembers::MapCountOptions(NativeCountOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FGetMembers::MapGetByIndexOptions(
    NativeGetByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.MemberIndex = Index;
}

void FGetMembers::MapGetByIndexResult(
    ResultEntry &Result,
    const NativeGetByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result = NativeResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()