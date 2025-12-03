// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyModification/AddAttribute.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(AddAttribute);

void FAddAttribute::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.Attribute = (*Options.AttributeData).ToLobbyViaAllocator(Allocator);
    NativeOptions.Visibility = *Options.Visibility;
}

}

REDPOINT_EOS_CODE_GUARD_END()