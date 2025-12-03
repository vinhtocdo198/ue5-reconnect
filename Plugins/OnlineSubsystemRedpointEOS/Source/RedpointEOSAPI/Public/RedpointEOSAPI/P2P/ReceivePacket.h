// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/P2P/SocketId.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/SyncGetTwoInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FReceivePacket
{
private:
    typedef EOS_HP2P NativeHandle;
    typedef EOS_P2P_ReceivePacketOptions NativeOptions;
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;

    static FORCEINLINE int NativeOptionsVersion()
    {
        return EOS_P2P_RECEIVEPACKET_API_LATEST;
    }

    static FORCEINLINE EOS_EResult NativeFunction(
        NativeHandle Handle,
        const NativeOptions *Options,
        EOS_ProductUserId *OutPeerId,
        EOS_P2P_SocketId *OutSocketId,
        uint8_t *OutChannel,
        void *OutData,
        uint32_t *OutBytesWritten)
    {
        return EOS_P2P_ReceivePacket(Handle, Options, OutPeerId, OutSocketId, OutChannel, OutData, OutBytesWritten);
    }

    static FORCEINLINE const TCHAR *StatCallName()
    {
        return TEXT("RedpointEOS/API/EOS_P2P_ReceivePacket");
    }

    static FORCEINLINE const TCHAR *LogCallName()
    {
        return TEXT("EOS_P2P_ReceivePacket");
    }

public:
    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<uint32> MaxDataSizeBytes;
        ParamHelpers::TRequired<TOptional<uint8>> RequestedChannel;
    };

public:
    static EOS_EResult Execute(
        const FPlatformHandle &InHandle,
        const Options &InOptions,
        EOS_ProductUserId &OutPeerId,
        FSocketId &OutSocketId,
        uint8 &OutChannel,
        void *OutData,
        uint32_t &OutBytesWritten);
    static EOS_EResult Execute(
        NativeHandle InHandle,
        const Options &InOptions,
        EOS_ProductUserId &OutPeerId,
        FSocketId &OutSocketId,
        uint8 &OutChannel,
        void *OutData,
        uint32_t &OutBytesWritten);

private:
    static void MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator);
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()