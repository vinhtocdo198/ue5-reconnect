// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketRoleRemote.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/P2P/SendPacket.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRoleListeningState.h"
#include "RedpointEOSNetworking/Socket/SocketRoleRemoteState.h"
#include "RedpointEOSNetworking/Socket/SocketTracing.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(767262975, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;

FName FSocketRoleRemote::GetRoleName() const
{
    return FName(TEXT("Remote"));
}

bool FSocketRoleRemote::Close(FSocketRedpointEOS &Socket) const
{
    // Get our parent listening socket.
    auto State = this->GetState<FSocketRoleRemoteState>(Socket);
    TSharedPtr<FSocketRedpointEOS> ListeningSocket = State->RemoteSocketParent.Pin();
    if (ListeningSocket.IsValid())
    {
        // Notify the listening socket that a remote connection is closing.
        ListeningSocket->OnConnectionClosed.ExecuteIfBound(
            ListeningSocket.ToSharedRef(),
            Socket.AsSharedRedpointEOS(),
            TEXT("FSocketRoleRemote::Close"));

        // Remove ourselves from the listening socket.
        auto ListeningRoleState = StaticCastSharedRef<FSocketRoleListeningState>(ListeningSocket->RoleState);
        for (int i = ListeningRoleState->RemoteSockets.Num() - 1; i >= 0; i--)
        {
            if (ListeningRoleState->RemoteSockets[i].Pin().Get() == &Socket)
            {
                ListeningRoleState->RemoteSockets.RemoveAt(i);
            }
        }
    }

    // Sockets are automatically removed from subsystem routing when Close() is called on FSocketRedpointEOS.

    return true;
}

bool FSocketRoleRemote::Connect(FSocketRedpointEOS &Socket, const FInternetAddrRedpointEOS &InDestAddr) const
{
    check(false /* Connect can not be called on a remote socket */);
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Connect called on a remote socket!"));
    return false;
}

bool FSocketRoleRemote::Listen(FSocketRedpointEOS &Socket, int32 MaxBacklog) const
{
    check(false /* Listen can not be called on a remote socket */);
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Listen called on a remote socket!"));
    return false;
}

bool FSocketRoleRemote::Accept(
    FSocketRedpointEOS &Socket,
    TSharedRef<FSocketRedpointEOS> InRemoteSocket,
    const FInternetAddrRedpointEOS &InRemoteAddr) const
{
    check(false /* Accept can not be called on a remote socket */);
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("Listen called on a remote socket!"));
    return false;
}

bool FSocketRoleRemote::HasPendingData(FSocketRedpointEOS &Socket, uint32 &PendingDataSize) const
{
    if (!Socket.ReceivedPacketsQueue.IsEmpty())
    {
        PendingDataSize = static_cast<uint32>(Socket.ReceivedPacketsQueue.Peek()->Get()->GetDataLen());
        return true;
    }
    return false;
}

bool FSocketRoleRemote::SendTo(
    FSocketRedpointEOS &Socket,
    const uint8 *Data,
    int32 Count,
    int32 &BytesSent,
    const FInternetAddrRedpointEOS &Destination) const
{
    auto State = this->GetState<FSocketRoleRemoteState>(Socket);
    TSharedPtr<FSocketRedpointEOS> ParentSocket = State->RemoteSocketParent.Pin();
    check(ParentSocket.IsValid());

    // Ensure that the address we are sending to is our remote address.
    check(Destination == (const FInternetAddr &)(*State->RemoteAddr));

    auto Opts = FSendPacket::Options{
        GetProductUserId(ParentSocket->BindAddress->GetUserId()),
        GetProductUserId(State->RemoteAddr->GetUserId()),
        State->RemoteAddr->GetSymmetricSocketId(),
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

bool FSocketRoleRemote::RecvFrom(
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
        int32 SizeToRead = FMath::Min(BufferSize, ReceivedPacket->GetDataLen());
        FMemory::Memcpy(Data, ReceivedPacket->GetData(), static_cast<SIZE_T>(SizeToRead));
        BytesRead = SizeToRead;
        Source.SetRawIp(this->GetState<FSocketRoleRemoteState>(Socket)->RemoteAddr->GetRawIp());
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
        RecordPacketTiming(ReceivedPacket->GetTiming());
#endif
        return true;
    }

    return false;
}

bool FSocketRoleRemote::GetPeerAddress(FSocketRedpointEOS &Socket, FInternetAddrRedpointEOS &OutAddr) const
{
    OutAddr.SetRawIp(this->GetState<FSocketRoleRemoteState>(Socket)->RemoteAddr->GetRawIp());
    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()