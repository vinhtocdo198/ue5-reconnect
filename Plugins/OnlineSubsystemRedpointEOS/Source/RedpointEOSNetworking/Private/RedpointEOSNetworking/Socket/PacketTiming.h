// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
#include "HAL/IConsoleManager.h"
#include "Misc/DateTime.h"
#include "RedpointEOSAPI/P2P/SocketId.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSNetworking/Socket/ConsoleVariables.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3354339269, Redpoint::EOS::Networking::Socket)
{
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Networking::Socket;
#endif

struct FPacketTiming
{
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
    FPacketTiming(
        const EOS_ProductUserId &InSourceUserId,
        const EOS_ProductUserId &InDestinationUserId,
        const FSocketId &InSymmetricSocketId,
        uint8_t InSymmetricChannel,
        int64 SentTicks,
        const FDateTime &ReceivedAt,
        int32 InSize);
#else
    FPacketTiming() = default;
#endif
    UE_NONCOPYABLE(FPacketTiming);
    ~FPacketTiming() = default;

#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
    bool bHasTimingData;

    FString SourceUserId;

    FString DestinationUserId;

    FString SymmetricSocketId;

    uint8 SymmetricChannel;

    FDateTime Sent;

    FDateTime ReceivedIntoQueue;

    FDateTime PulledFromQueue;

    int32 Size;
#endif
};

#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
void RecordPacketTiming(const TSharedPtr<FPacketTiming> &InPacketTiming);
#endif

}

namespace Redpoint::EOS::Networking::Socket
{
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
REDPOINT_EOS_FILE_NS_EXPORT(3354339269, Redpoint::EOS::Networking::Socket, CVarEnablePacketTiming)
#endif
REDPOINT_EOS_FILE_NS_EXPORT(3354339269, Redpoint::EOS::Networking::Socket, FPacketTiming)
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
REDPOINT_EOS_FILE_NS_EXPORT(3354339269, Redpoint::EOS::Networking::Socket, RecordPacketTiming)
#endif
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
