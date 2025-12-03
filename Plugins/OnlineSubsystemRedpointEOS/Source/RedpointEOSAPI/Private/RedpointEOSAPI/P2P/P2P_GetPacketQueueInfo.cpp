// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/GetPacketQueueInfo.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/SyncGet.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_SYNC_GET_IMPL(GetPacketQueueInfo);

void FGetPacketQueueInfo::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FGetPacketQueueInfo::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.IncomingPacketQueueMaxSizeBytes = NativeResult.IncomingPacketQueueMaxSizeBytes;
    Result.IncomingPacketQueueCurrentSizeBytes = NativeResult.IncomingPacketQueueCurrentSizeBytes;
    Result.IncomingPacketQueueCurrentPacketCount = NativeResult.IncomingPacketQueueCurrentPacketCount;
    Result.OutgoingPacketQueueMaxSizeBytes = NativeResult.OutgoingPacketQueueMaxSizeBytes;
    Result.OutgoingPacketQueueCurrentSizeBytes = NativeResult.OutgoingPacketQueueCurrentSizeBytes;
    Result.OutgoingPacketQueueCurrentPacketCount = NativeResult.OutgoingPacketQueueCurrentPacketCount;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()