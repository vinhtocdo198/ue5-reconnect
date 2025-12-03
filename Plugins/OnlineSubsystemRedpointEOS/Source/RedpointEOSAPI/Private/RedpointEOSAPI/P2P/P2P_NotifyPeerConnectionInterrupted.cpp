// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/NotifyPeerConnectionInterrupted.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyPeerConnectionInterrupted);

void FNotifyPeerConnectionInterrupted::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    if ((*Options.SocketId).IsSet())
    {
        NativeOptions.SocketId = AllocateSocketId(Allocator, **Options.SocketId);
    }
    else
    {
        NativeOptions.SocketId = nullptr;
    }
}

void FNotifyPeerConnectionInterrupted::MapResult(
    Result &Result,
    const NativeResult &NativeResult,
    NativeConverter &Converter)
{
    Result.LocalUserId = NativeResult.LocalUserId;
    Result.RemoteUserId = NativeResult.RemoteUserId;
    Result.SocketId =
        NativeResult.SocketId == nullptr ? TEXT("") : Converter.FromAnsi(NativeResult.SocketId->SocketName);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()