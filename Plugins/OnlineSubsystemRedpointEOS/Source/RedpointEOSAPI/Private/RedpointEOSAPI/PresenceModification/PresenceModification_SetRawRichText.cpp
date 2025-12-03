// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PresenceModification/SetRawRichText.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PresenceModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetRawRichText);

void FSetRawRichText::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.RichText = Allocator.AsUtf8(*Options.RichText);
}

}

REDPOINT_EOS_CODE_GUARD_END()