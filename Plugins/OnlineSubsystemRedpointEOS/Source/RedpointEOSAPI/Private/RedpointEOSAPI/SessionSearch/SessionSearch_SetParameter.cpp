// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionSearch/SetParameter.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionSearch
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetParameter);

void FSetParameter::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.Parameter = (*Options.AttributeData).ToSessionViaAllocator(Allocator);
    NativeOptions.ComparisonOp = *Options.ComparisonOp;
}

}

REDPOINT_EOS_CODE_GUARD_END()