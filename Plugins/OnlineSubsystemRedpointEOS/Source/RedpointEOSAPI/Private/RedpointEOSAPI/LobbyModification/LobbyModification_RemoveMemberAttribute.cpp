// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyModification/RemoveMemberAttribute.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(RemoveMemberAttribute);

void FRemoveMemberAttribute::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.Key = Allocator.AsUtf8(*Options.AttributeKey);
}

}

REDPOINT_EOS_CODE_GUARD_END()