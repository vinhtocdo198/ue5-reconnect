// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketRoleListening.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/Socket/ListenManager.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRoleListeningState.h"
#include "RedpointEOSNetworking/Socket/SocketRoleRemote.h"
#include "RedpointEOSNetworking/Socket/SocketRoleRemoteState.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1773620347, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;

FName FSocketRoleListening::GetRoleName() const
{
    return FName(TEXT("Listening"));
}

bool FSocketRoleListening::Close(FSocketRedpointEOS &Socket) const
{
    // Ensure the socket is bound.
    if (!Socket.BindAddress.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Warning,
            TEXT("%d: Attempted to call FSocketRoleListening::Close but the socket has not been locally bound."),
            Socket.SocketMemoryId);
        return false;
    }

    return Socket.SocketSubsystem->ListenManager->Remove(Socket, *Socket.BindAddress);
}

bool FSocketRoleListening::Connect(FSocketRedpointEOS &Socket, const FInternetAddrRedpointEOS &InDestAddr) const
{
    // Connect can not be called on Listening sockets.
    return false;
}

bool FSocketRoleListening::Listen(FSocketRedpointEOS &Socket, int32 MaxBacklog) const
{
    // Ensure the socket is bound.
    if (!Socket.BindAddress.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("%d: Attempted to call FSocketRoleListening::Listen but the socket has not been locally bound."),
            Socket.SocketMemoryId);
        return false;
    }

    return Socket.SocketSubsystem->ListenManager->Add(Socket, *Socket.BindAddress);
}

bool FSocketRoleListening::Accept(
    FSocketRedpointEOS &Socket,
    TSharedRef<FSocketRedpointEOS> InRemoteSocket,
    const FInternetAddrRedpointEOS &InRemoteAddr) const
{
    // Ensure the socket is bound.
    if (!Socket.BindAddress.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("%d: Attempted to call FSocketRoleListening::Accept but the socket has not been locally bound."),
            Socket.SocketMemoryId);
        return false;
    }

    this->GetState<FSocketRoleListeningState>(Socket)->RemoteSockets.Add(InRemoteSocket);

    auto RemoteState = MakeShared<FSocketRoleRemoteState>();
    RemoteState->RemoteAddr = MakeShared<FInternetAddrRedpointEOS>();
    RemoteState->RemoteAddr->SetRawIp(InRemoteAddr.GetRawIp());
    RemoteState->RemoteSocketParent = Socket.AsSharedRedpointEOS();
    checkf(IsProductUserId(RemoteState->RemoteAddr->GetUserId()), TEXT("User ID for incoming address must be valid!"));

    UE_LOG(
        LogRedpointEOSSocketLifecycle,
        Verbose,
        TEXT("%d: Socket moved to: RoleInstance_Remote"),
        InRemoteSocket->SocketMemoryId);
    InRemoteSocket->RoleState = RemoteState;
    InRemoteSocket->Role = Socket.SocketSubsystem->RoleInstance_Remote;

    InRemoteSocket->UpdateSocketKey(FSocketConnectionMapKey(
        Socket.BindAddress->GetUserId(),
        InRemoteAddr.GetUserId(),
        InRemoteAddr.GetSymmetricSocketId(),
        InRemoteAddr.GetSymmetricChannel()));

    auto PendingResetKey = InRemoteAddr.ToString(true);
    if (Socket.SocketSubsystem->PendingResetIds.Contains(PendingResetKey))
    {
        RemoteState->SocketAssignedResetId = Socket.SocketSubsystem->PendingResetIds[PendingResetKey];
        Socket.SocketSubsystem->PendingResetIds.Remove(PendingResetKey);

        UE_LOG(
            LogRedpointEOSSocket,
            Verbose,
            TEXT("%s: When remote socket was opened, found a pending reset ID of %u to use."),
            *InRemoteSocket->SocketKey->ToString(false),
            RemoteState->SocketAssignedResetId);
    }
    else
    {
        // We have received data before the reset ID packet.
        RemoteState->SocketAssignedResetId = 0;

        UE_LOG(
            LogRedpointEOSSocket,
            Verbose,
            TEXT(
                "%s: When remote socket was opened, no pending reset ID was found. Expect one to be assigned shortly."),
            *InRemoteSocket->SocketKey->ToString(false));
    }

    return true;
}

bool FSocketRoleListening::HasPendingData(FSocketRedpointEOS &Socket, uint32 &PendingDataSize) const
{
    for (const auto &RemoteSocketWk : this->GetState<FSocketRoleListeningState>(Socket)->RemoteSockets)
    {
        if (auto RemoteSocket = RemoteSocketWk.Pin())
        {
            if (!RemoteSocket->ReceivedPacketsQueue.IsEmpty())
            {
                PendingDataSize = static_cast<uint32>(RemoteSocket->ReceivedPacketsQueue.Peek()->Get()->GetDataLen());
                return true;
            }
        }
    }

    return false;
}

bool FSocketRoleListening::SendTo(
    FSocketRedpointEOS &Socket,
    const uint8 *Data,
    int32 Count,
    int32 &BytesSent,
    const FInternetAddrRedpointEOS &Destination) const
{
    for (const auto &RemoteSocketWk : this->GetState<FSocketRoleListeningState>(Socket)->RemoteSockets)
    {
        if (auto RemoteSocket = RemoteSocketWk.Pin())
        {
            TSharedPtr<FInternetAddrRedpointEOS> PeerAddress = MakeShared<FInternetAddrRedpointEOS>();
            if (RemoteSocket->GetPeerAddress(*PeerAddress))
            {
                if ((const FInternetAddr &)(*PeerAddress) == Destination)
                {
                    RemoteSocket->Role->SendTo(*RemoteSocket, Data, Count, BytesSent, Destination);
                    return true;
                }
            }
        }
    }

    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("SentTo on a listening socket didn't match any connected client!"));
    return false;
}

bool FSocketRoleListening::RecvFrom(
    FSocketRedpointEOS &Socket,
    uint8 *Data,
    int32 BufferSize,
    int32 &BytesRead,
    FInternetAddr &Source,
    ESocketReceiveFlags::Type Flags) const
{
    for (const auto &RemoteSocketWk : this->GetState<FSocketRoleListeningState>(Socket)->RemoteSockets)
    {
        if (auto RemoteSocket = RemoteSocketWk.Pin())
        {
            if (!RemoteSocket->ReceivedPacketsQueue.IsEmpty())
            {
                return RemoteSocket->Role->RecvFrom(*RemoteSocket, Data, BufferSize, BytesRead, Source, Flags);
            }
        }
    }

    return false;
}

bool FSocketRoleListening::GetPeerAddress(class FSocketRedpointEOS &Socket, FInternetAddrRedpointEOS &OutAddr) const
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("GetPeerAddress called on a listening socket!"));
    return false;
}

const TArray<TWeakPtr<FSocketRedpointEOS>> FSocketRoleListening::GetOwnedSockets(class FSocketRedpointEOS &Socket) const
{
    return this->GetState<FSocketRoleListeningState>(Socket)->RemoteSockets;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
