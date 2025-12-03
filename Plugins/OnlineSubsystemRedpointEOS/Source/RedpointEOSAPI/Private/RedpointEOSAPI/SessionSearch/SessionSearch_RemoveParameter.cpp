// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionSearch/RemoveParameter.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionSearch
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(RemoveParameter);

void FRemoveParameter::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.Key = Allocator.AsUtf8(*Options.AttributeKey);
    NativeOptions.ComparisonOp = *Options.ComparisonOp;
}

}

REDPOINT_EOS_CODE_GUARD_END()