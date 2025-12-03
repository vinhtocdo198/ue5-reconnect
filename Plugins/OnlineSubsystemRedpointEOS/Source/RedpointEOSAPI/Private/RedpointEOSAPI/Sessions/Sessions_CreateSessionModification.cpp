// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/CreateSessionModification.h"

#include "RedpointEOSAPI/Templates/Impl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_IMPL(CreateSessionModification);

void FCreateSessionModification::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.SessionName = Allocator.AsAnsi(*Options.SessionName);
    NativeOptions.BucketId = Allocator.AsAnsi(*Options.BucketId);
    NativeOptions.MaxPlayers = *Options.MaxPlayers;
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.bPresenceEnabled = *Options.bPresenceEnabled ? EOS_TRUE : EOS_FALSE;
    NativeOptions.SessionId =
        Allocator.AsAnsi(*Options.SessionId, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
    NativeOptions.bSanctionsEnabled = *Options.bSanctionsEnabled ? EOS_TRUE : EOS_FALSE;
    NativeOptions.AllowedPlatformIds = reinterpret_cast<const uint32_t *>(Allocator.AllocateArray(
        *Options.AllowedPlatformIds,
        NativeOptions.AllowedPlatformIdsCount,
        Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty));
}

}

REDPOINT_EOS_CODE_GUARD_END()