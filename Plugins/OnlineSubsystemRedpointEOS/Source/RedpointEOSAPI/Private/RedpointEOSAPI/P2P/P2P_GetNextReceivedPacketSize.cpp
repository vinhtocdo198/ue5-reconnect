// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/GetNextReceivedPacketSize.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/SyncGet.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_SYNC_GET_IMPL(GetNextReceivedPacketSize);

void FGetNextReceivedPacketSize::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    if ((*Options.RequestedChannel).IsSet())
    {
        uint8_t &RequestedChannelPtr = Allocator.Allocate<uint8_t>();
        RequestedChannelPtr = **Options.RequestedChannel;
        NativeOptions.RequestedChannel = &RequestedChannelPtr;
    }
    else
    {
        NativeOptions.RequestedChannel = nullptr;
    }
}

void FGetNextReceivedPacketSize::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result = NativeResult;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()