// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyDetails/CopyAttributes.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_IMPL(CopyAttributes);

void FCopyAttributes::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FCopyAttributes::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.AttrIndex = Index;
}

void FCopyAttributes::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.Data = LobbySessionsShared::FAttributeData(*NativeResult.Data, Converter);
    Result.bPublic = NativeResult.Visibility == EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
}

}

REDPOINT_EOS_CODE_GUARD_END()