// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyDetails/CopyMemberAttributes.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_IMPL(CopyMemberAttributes);

void FCopyMemberAttributes::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

void FCopyMemberAttributes::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.TargetUserId = *Options.TargetUserId;
    NativeOptions.AttrIndex = Index;
}

void FCopyMemberAttributes::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.Data = LobbySessionsShared::FAttributeData(*NativeResult.Data, Converter);
    Result.bPublic = NativeResult.Visibility == EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC;
}

}

REDPOINT_EOS_CODE_GUARD_END()