// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketRoleClient.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/P2P/NotifyPeerConnectionClosed.h"
#include "RedpointEOSAPI/P2P/SendPacket.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/Socket/P2PChannel.h"
#include "RedpointEOSNetworking/Socket/PacketTiming.h"
#include "RedpointEOSNetworking/Socket/RawPacket.h"
#include "RedpointEOSNetworking/Socket/SocketConnectionMapKey.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRoleClientState.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketTracing.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3448274137, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;

void FSocketRoleClient::OnRemoteConnectionClosed(
    const TSharedRef<FSocketRedpointEOS> &Socket,
    const FNotifyPeerConnectionClosed::Result &Info) const
{
    const auto InfoKey =
        FSocketConnectionMapKey(GetAccountId(Info.LocalUserId), GetAccountId(Info.RemoteUserId), Info.SocketId, 0);
    if (!Socket->SocketKey.IsValid() || Socket->SocketKey->IsIdenticalExcludingChannel(InfoKey))
    {
        Socket->OnConnectionClosed.ExecuteIfBound(Socket, Socket, TEXT("FSocketRoleClient::OnRemoteConnectionClosed"));
        Socket->bAlreadyClosedByRemoteHost = true;
        Socket->Close();
    }
}

FName FSocketRoleClient::GetRoleName() const
{
    return FName(TEXT("Client"));
}

bool FSocketRoleClient::Close(FSocketRedpointEOS &Socket) const
{
    // Sockets are automatically removed from subsystem routing when Close() is called on FSocketRedpointEOS.
    return true;
}

bool FSocketRoleClient::Listen(FSocketRedpointEOS &Socket, int32 MaxBacklog) const
{
    // Listen can not be called on client sockets.
    check(false);
    return false;
}

bool FSocketRoleClient::Accept(
    class FSocketRedpointEOS &Socket,
    TSharedRef<FSocketRedpointEOS> InRemoteSocket,
    const FInternetAddrRedpointEOS &InRemoteAddr) const
{
    // Accept can not be called on client sockets.
    check(false);
    return false;
}

bool FSocketRoleClient::Connect(FSocketRedpointEOS &Socket, const FInternetAddrRedpointEOS &InDestAddr) const
{
    auto State = this->GetState<FSocketRoleClientState>(Socket);
    State->RemoteAddr = MakeShared<FInternetAddrRedpointEOS>();
    State->RemoteAddr->SetRawIp(InDestAddr.GetRawIp());
    State->bDidSendChannelReset = false;
    State->SocketResetId = Socket.SocketSubsystem->GetResetIdForOutboundConnection();
    check(State->RemoteAddr->GetSymmetricChannel() != 0);

    // Ensure the socket is bound.
    if (!Socket.BindAddress.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("%d: Attempted to call FSocketRoleClient::Connect but the socket has not been locally bound."),
            Socket.SocketMemoryId);
        return false;
    }

    Socket.UpdateSocketKey(FSocketConnectionMapKey(
        Socket.BindAddress->GetUserId(),
        State->RemoteAddr->GetUserId(),
        State->RemoteAddr->GetSymmetricSocketId(),
        State->RemoteAddr->GetSymmetricChannel()));

    UE_LOG(
        LogRedpointEOSSocket,
        Verbose,
        TEXT("%s: Client socket opened with reset ID %u."),
        *Socket.SocketKey->ToString(false),
        State->SocketResetId);

    // Reset connections (via reset packets) are handled in the socket subsystem, but full
    // disconnections are handled in the listen manager. Since the listen manager only manages
    // non-client sockets, we need to listen for full connection close events for clients here
    // so that we can appropriately call Close and fire OnConnectionClosed.
    {
        State->OnClosedEvent = FNotifyPeerConnectionClosed::Register(
            Socket.PlatformHandle,
            FNotifyPeerConnectionClosed::Options{
                GetProductUserId(Socket.BindAddress->GetUserId()),
                TOptional<FSocketId>(Socket.BindAddress->GetSymmetricSocketId())},
            FNotifyPeerConnectionClosed::EventDelegate::CreateSPLambda(
                this,
                [this, WeakSocket = TWeakPtr<FSocketRedpointEOS>(Socket.AsSharedRedpointEOS())](
                    const FNotifyPeerConnectionClosed::Result &Info) {
                    auto PinnedWeakSocket = WeakSocket.Pin();
                    if (PinnedWeakSocket.IsValid())
                    {
                        this->OnRemoteConnectionClosed(PinnedWeakSocket.ToSharedRef(), Info);
                    }
                }));
    }

    return true;
}

bool FSocketRoleClient::HasPendingData(FSocketRedpointEOS &Socket, uint32 &PendingDataSize) const
{
    if (!Socket.ReceivedPacketsQueue.IsEmpty())
    {
        PendingDataSize = static_cast<uint32>(Socket.ReceivedPacketsQueue.Peek()->Get()->GetDataLen());
        return true;
    }
    return false;
}

bool FSocketRoleClient::SendTo(
    FSocketRedpointEOS &Socket,
    const uint8 *Data,
    int32 Count,
    int32 &BytesSent,
    const FInternetAddrRedpointEOS &Destination) const
{
    auto State = this->GetState<FSocketRoleClientState>(Socket);
    check(State->RemoteAddr.IsValid());

    // Ensure that the address we are sending to is our remote address.
    check(Destination == (const FInternetAddr &)(*State->RemoteAddr));

    // Ensure the socket is bound.
    if (!Socket.BindAddress.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("%d: Attempted to call FSocketRoleClient::SendTo but the socket has not been locally bound."),
            Socket.SocketMemoryId);
        return false;
    }

    // Send the channel reset if it hasn't already been done.
    if (!State->bDidSendChannelReset)
    {
        FSocketId SocketId = State->RemoteAddr->GetSymmetricSocketId();

        check(State->SocketResetId != 0);

        uint8 ChannelToResetAndId[5] = {
            State->RemoteAddr->GetSymmetricChannel(),
            reinterpret_cast<uint8 *>(&State->SocketResetId)[0],
            reinterpret_cast<uint8 *>(&State->SocketResetId)[1],
            reinterpret_cast<uint8 *>(&State->SocketResetId)[2],
            reinterpret_cast<uint8 *>(&State->SocketResetId)[3],
        };

        auto Opts = FSendPacket::Options{
            GetProductUserId(Socket.BindAddress->GetUserId()),
            GetProductUserId(State->RemoteAddr->GetUserId()),
            SocketId,
            P2PChannelIdControl,
            5,
            ChannelToResetAndId,
            true,
            EOS_EPacketReliability::EOS_PR_ReliableOrdered,
            false};

        LogSocketPacket(Opts);

        auto SendResult = FSendPacket::Execute(Socket.PlatformHandle, Opts);

        if (SendResult == EOS_EResult::EOS_Success)
        {
            State->bDidSendChannelReset = true;
        }
        else
        {
            UE_LOG(LogRedpointEOSSocket, Warning, TEXT("Unable to send channel reset message."));
            BytesSent = 0;
            return false;
        }
    }

    // Send the actual packet.
    {
        FSocketId SocketId = State->RemoteAddr->GetSymmetricSocketId();

        auto Opts = FSendPacket::Options{
            GetProductUserId(Socket.BindAddress->GetUserId()),
            GetProductUserId(State->RemoteAddr->GetUserId()),
            SocketId,
            State->RemoteAddr->GetSymmetricChannel(),
            static_cast<uint32>(Count),
            Data,
            Socket.bEnableReliableSending,
            Socket.bEnableReliableSending ? EOS_EPacketReliability::EOS_PR_ReliableOrdered
                                          : EOS_EPacketReliability::EOS_PR_UnreliableUnordered,
            false};

        LogSocketPacket(Opts);

        auto SendResult = FSendPacket::Execute(Socket.PlatformHandle, Opts);

        if (SendResult == EOS_EResult::EOS_Success)
        {
            BytesSent = Count;
            return true;
        }

        BytesSent = 0;
        return false;
    }
}

bool FSocketRoleClient::RecvFrom(
    FSocketRedpointEOS &Socket,
    uint8 *Data,
    int32 BufferSize,
    int32 &BytesRead,
    FInternetAddr &Source,
    ESocketReceiveFlags::Type Flags) const
{
    TSharedPtr<FRawPacket> ReceivedPacket;
    if (Socket.ReceivedPacketsQueue.Dequeue(ReceivedPacket))
    {
        Socket.ReceivedPacketsQueueCount--;
        check(ReceivedPacket.IsValid());
        int32 SizeToRead = FMath::Min(BufferSize, ReceivedPacket->GetDataLen());
        FMemory::Memcpy(Data, ReceivedPacket->GetData(), static_cast<SIZE_T>(SizeToRead));
        BytesRead = SizeToRead;
        Source.SetRawIp(this->GetState<FSocketRoleClientState>(Socket)->RemoteAddr->GetRawIp());
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
        RecordPacketTiming(ReceivedPacket->GetTiming());
#endif
        return true;
    }

    return false;
}

bool FSocketRoleClient::GetPeerAddress(class FSocketRedpointEOS &Socket, FInternetAddrRedpointEOS &OutAddr) const
{
    OutAddr.SetRawIp(this->GetState<FSocketRoleClientState>(Socket)->RemoteAddr->GetRawIp());
    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
