// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionDetails/CopySessionAttributes.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionDetails
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_IMPL(CopySessionAttributes);

void FCopySessionAttributes::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FCopySessionAttributes::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.AttrIndex = Index;
}

void FCopySessionAttributes::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.Data = LobbySessionsShared::FAttributeData(*NativeResult.Data, Converter);
    Result.AdvertisementType = NativeResult.AdvertisementType;
}

}

REDPOINT_EOS_CODE_GUARD_END()