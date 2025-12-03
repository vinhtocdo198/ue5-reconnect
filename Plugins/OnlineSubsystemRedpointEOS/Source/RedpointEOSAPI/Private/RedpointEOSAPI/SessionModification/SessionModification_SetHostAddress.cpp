// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionModification/SetHostAddress.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetHostAddress);

void FSetHostAddress::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.HostAddress = Allocator.AsAnsi(*Options.HostAddress);
}

}

REDPOINT_EOS_CODE_GUARD_END()