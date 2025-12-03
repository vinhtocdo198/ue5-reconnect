// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyModification/SetBucketId.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetBucketId);

void FSetBucketId::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.BucketId = Allocator.AsAnsi(*Options.BucketId);
}

}

REDPOINT_EOS_CODE_GUARD_END()