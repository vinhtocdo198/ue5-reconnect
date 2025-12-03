// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionModification/AddAttribute.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(AddAttribute);

void FAddAttribute::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.SessionAttribute = (*Options.AttributeData).ToSessionViaAllocator(Allocator);
    NativeOptions.AdvertisementType = *Options.AdvertisementType;
}

}

REDPOINT_EOS_CODE_GUARD_END()