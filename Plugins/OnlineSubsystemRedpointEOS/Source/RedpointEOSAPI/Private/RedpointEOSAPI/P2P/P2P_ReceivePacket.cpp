// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/ReceivePacket.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

EOS_EResult FReceivePacket::Execute(
    const FPlatformHandle &InHandle,
    const Options &InOptions,
    EOS_ProductUserId &OutPeerId,
    FSocketId &OutSocketId,
    uint8 &OutChannel,
    void *OutData,
    uint32_t &OutBytesWritten)
{
    return Execute(
        InHandle->Get<NativeHandle>(),
        InOptions,
        OutPeerId,
        OutSocketId,
        OutChannel,
        OutData,
        OutBytesWritten);
}

EOS_EResult FReceivePacket::Execute(
    NativeHandle InHandle,
    const Options &InOptions,
    EOS_ProductUserId &OutPeerId,
    FSocketId &OutSocketId,
    uint8 &OutChannel,
    void *OutData,
    uint32_t &OutBytesWritten)
{
    checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));
    
    DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);
    SCOPE_CYCLE_COUNTER(STAT_Call);

    UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallName());

    if (InHandle == nullptr)
    {
        UE_LOG(
            LogRedpointEOSAPI,
            VeryVerbose,
            TEXT("[complete] %s"),
            *ConvertError(
                 LogCallName(),
                 TEXT("The platform instance has been shutdown, so SDK calls can not be made."),
                 EOS_EResult::EOS_NoConnection)
                 .ToLogString());

        return EOS_EResult::EOS_NoConnection;
    }

    TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();

    NativeOptions _NativeOptions = {};
    _NativeOptions.ApiVersion = NativeOptionsVersion();
    MapOptions(_NativeOptions, InOptions, *Allocator);

    OutPeerId = nullptr;
    OutSocketId = TEXT("");
    OutChannel = 0;
    OutBytesWritten = 0;

    EOS_P2P_SocketId SocketId = {};

    EOS_EResult ResultCode =
        NativeFunction(InHandle, &_NativeOptions, &OutPeerId, &SocketId, &OutChannel, OutData, &OutBytesWritten);

    if (ResultCode == EOS_EResult::EOS_Success)
    {
        // Ensure that this is null-terminated.
        SocketId.SocketName[EOS_P2P_SOCKETID_SOCKETNAME_SIZE - 1] = '\0';

        OutSocketId = (*MakeShared<NativeConverter>()).FromAnsi(SocketId.SocketName);
    }

    UE_LOG(
        LogRedpointEOSAPI,
        VeryVerbose,
        TEXT("[complete] %s"),
        *ConvertError(LogCallName(), TEXT("Receive packet call completed."), ResultCode).ToLogString());
    return ResultCode;
};

void FReceivePacket::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.MaxDataSizeBytes = *Options.MaxDataSizeBytes;
    if ((*Options.RequestedChannel).IsSet())
    {
        auto &RequestedChannel = Allocator.Allocate<uint8_t>();
        RequestedChannel = **Options.RequestedChannel;
        NativeOptions.RequestedChannel = &RequestedChannel;
    }
    else
    {
        NativeOptions.RequestedChannel = nullptr;
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()