// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/NotifyIncomingPacketQueueFull.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyIncomingPacketQueueFull);

void FNotifyIncomingPacketQueueFull::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyIncomingPacketQueueFull::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.PacketQueueMaxSizeBytes = NativeResult.PacketQueueMaxSizeBytes;
    Result.PacketQueueCurrentSizeBytes = NativeResult.PacketQueueCurrentSizeBytes;
    Result.OverflowPacketLocalUserId = NativeResult.OverflowPacketLocalUserId;
    Result.OverflowPacketChannel = NativeResult.OverflowPacketChannel;
    Result.OverflowPacketSizeBytes = NativeResult.OverflowPacketSizeBytes;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()