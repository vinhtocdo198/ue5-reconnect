// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/NetConnectionImpl.h"

#include "IPAddress.h"
#include "Net/DataChannel.h"
#include "Net/NetworkProfiler.h"
#include "PacketHandler.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/NetDriverImpl.h"
#include "RedpointEOSNetworking/Stats.h"
#include "TimerManager.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2904893824, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::Networking;

FNetConnectionImpl::FNetConnectionImpl(
    TSoftObjectPtr<URedpointEOSNetDriver> InNetDriver,
    TSoftObjectPtr<URedpointEOSNetConnection> InNetConnection)
    : NetDriver(InNetDriver)
    , NetConnection(InNetConnection)
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    , Socket()
#endif
{
}

#if REDPOINT_EOS_ENABLE_SOCKET_LEVEL_NETWORK_TRACING

void FNetConnectionImpl::LogPacket(
    UNetDriver *NetDriver,
    const TSharedPtr<const FInternetAddr> &InRemoteAddr,
    bool bIncoming,
    const uint8 *Data,
    int32 CountBytes)
{
    UE_LOG(
        LogRedpointEOSNetworkTrace,
        Verbose,
        TEXT("'%s' %s '%s': %s"),
        *NetDriver->GetLocalAddr()->ToString(true),
        bIncoming ? TEXT("<-") : TEXT("->"),
        *(InRemoteAddr->ToString(true)),
        *FString::FromBlob(Data, static_cast<uint32>(CountBytes)));
}

#endif

void FNetConnectionImpl::ReceivePacketFromDriver(
    const TSharedPtr<FInternetAddr> &ReceivedAddr,
    uint8 *Buffer,
    int32 BufferSize)
{
    if (!this->NetConnection.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("%s: NetConnection not valid when this function was called."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    // We do not use the packet handler infrastructure, so just forward
    // the received packet straight into the networking stack.

#if REDPOINT_EOS_ENABLE_SOCKET_LEVEL_NETWORK_TRACING
    FNetConnectionImpl::LogPacket(this->NetDriver.Get(), ReceivedAddr, true, Buffer, BufferSize);
#endif
    this->NetConnection->ReceivedRawPacket(Buffer, BufferSize);
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FNetConnectionImpl::SetSocket(const TSharedRef<FSocketRedpointEOS> &InSocket)
{
    this->Socket = InSocket;
}

#endif

void FNetConnectionImpl::LowLevelSend(void *Data, int32 CountBits, FOutPacketTraits &Traits)
{
    if (!this->NetConnection.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("%s: NetConnection not valid when this function was called."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto SocketPinned = this->Socket.Pin();
    if (SocketPinned.IsValid())
    {
        if (SocketPinned->bClosed)
        {
            // This socket has been intentionally closed. We also need to close this connection.
            if (this->NetDriver.IsValid())
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Verbose,
                    TEXT("FNetConnectionImpl::LowLevelSend: Closing the connection as the underlying socket has been "
                         "closed (usually by the remote host)."));
                this->NetDriver->GetImpl()->RequestConnectionClose(this->NetConnection.Get());
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Warning,
                    TEXT(
                        "FNetConnectionImpl::LowLevelSend: Unable to inform network driver of connection close because "
                        "the network driver is not UEOSNetDriver."));
            }
            return;
        }

        const uint8 *SendData = reinterpret_cast<const uint8 *>(Data);

        // Process any packet modifiers
        if (this->NetConnection->Handler.IsValid() && !this->NetConnection->Handler->GetRawSend())
        {
            const ProcessedPacket ProcessedData =
                this->NetConnection->Handler->Outgoing(reinterpret_cast<uint8 *>(Data), CountBits, Traits);

            if (!ProcessedData.bError)
            {
                SendData = ProcessedData.Data;
                CountBits = ProcessedData.CountBits;
            }
            else
            {
                CountBits = 0;
            }
        }

        int32 BytesSent = 0;
        int32 BytesToSend = FMath::DivideAndRoundUp(CountBits, 8);

        if (BytesToSend > 0)
        {
#if REDPOINT_EOS_ENABLE_SOCKET_LEVEL_NETWORK_TRACING
            FNetConnectionImpl::LogPacket(
                this->NetDriver.Get(),
                this->NetConnection->RemoteAddr,
                true,
                SendData,
                BytesSent);
#endif
            if (!SocketPinned->SendTo(SendData, BytesToSend, BytesSent, *this->NetConnection->RemoteAddr))
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Warning,
                    TEXT("FNetConnectionImpl::LowLevelSend: Could not send %d data over socket to %s!"),
                    BytesToSend,
                    *this->NetConnection->RemoteAddr->ToString(true));
            }
            else
            {
                REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT(P2P, SentPackets);
                REDPOINT_EOS_DWORD_COUNTER_STAT_INCREMENT_BY(P2P, SentBytes, BytesToSend);
                REDPOINT_EOS_INT_COUNTER_INCREMENT(P2P, SentPackets);
                REDPOINT_EOS_INT_COUNTER_ADD(P2P, SentBytes, BytesToSend);

                NETWORK_PROFILER(GNetworkProfiler.FlushOutgoingBunches(this->NetConnection.Get()));
                NETWORK_PROFILER(GNetworkProfiler.TrackSocketSendTo(
                    SocketPinned->GetDescription(),
                    SendData,
                    (uint16)BytesToSend,
                    (uint16)this->NetConnection->NumPacketIdBits,
                    (uint16)this->NetConnection->NumBunchBits,
                    (uint16)this->NetConnection->NumAckBits,
                    (uint16)this->NetConnection->NumPaddingBits,
                    this->NetConnection.Get()));
            }
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FNetConnectionImpl::LowLevelSend: Could not send data because the socket is gone!"));
    }
#endif
}

FString FNetConnectionImpl::LowLevelGetRemoteAddress(bool bAppendPort)
{
    if (!this->NetConnection.IsValid() || this->NetConnection->RemoteAddr == nullptr)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT(
                "FNetConnectionImpl::LowLevelGetRemoteAddress called on connection without a valid RemoteAddr. Has the "
                "connection been closed?"));
        return TEXT("");
    }
    return this->NetConnection->RemoteAddr->ToString(true);
}

FString FNetConnectionImpl::LowLevelDescribe()
{
    return TEXT("EOS Net Connection");
}

}

REDPOINT_EOS_CODE_GUARD_END()