// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/SocketRoleNone.h"

#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/Socket/SocketConnectionMapKey.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRoleClient.h"
#include "RedpointEOSNetworking/Socket/SocketRoleClientState.h"
#include "RedpointEOSNetworking/Socket/SocketRoleListening.h"
#include "RedpointEOSNetworking/Socket/SocketRoleListeningState.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4168139954, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;

FName FSocketRoleNone::GetRoleName() const
{
    return FName(TEXT("None"));
}

bool FSocketRoleNone::Close(FSocketRedpointEOS &Socket) const
{
    // Sockets with the None role don't have anything to close.
    return false;
}

bool FSocketRoleNone::Listen(FSocketRedpointEOS &Socket, int32 MaxBacklog) const
{
    // Listen indicates that this should be a listening socket.

    auto OldState = Socket.RoleState;
    Socket.RoleState = MakeShared<FSocketRoleListeningState>();
    if (!Socket.SocketSubsystem->RoleInstance_Listening->Listen(Socket, MaxBacklog))
    {
        Socket.RoleState = OldState;
        return false;
    }

    UE_LOG(
        LogRedpointEOSSocketLifecycle,
        Verbose,
        TEXT("%d: Socket moved to: RoleInstance_Listening"),
        Socket.SocketMemoryId);
    Socket.Role = Socket.SocketSubsystem->RoleInstance_Listening;
    return true;
}

bool FSocketRoleNone::Accept(
    class FSocketRedpointEOS &Socket,
    TSharedRef<FSocketRedpointEOS> InRemoteSocket,
    const FInternetAddrRedpointEOS &InRemoteAddr) const
{
    // Accept can not be called on none sockets.
    return false;
}

bool FSocketRoleNone::Connect(FSocketRedpointEOS &Socket, const FInternetAddrRedpointEOS &InDestAddr) const
{
    // Connect indicates that this should be a client socket.

    auto OldState = Socket.RoleState;
    Socket.RoleState = MakeShared<FSocketRoleClientState>();
    if (!Socket.SocketSubsystem->RoleInstance_Client->Connect(Socket, InDestAddr))
    {
        Socket.RoleState = OldState;
        return false;
    }

    UE_LOG(
        LogRedpointEOSSocketLifecycle,
        Verbose,
        TEXT("%d: Socket moved to: RoleInstance_Client"),
        Socket.SocketMemoryId);
    Socket.Role = Socket.SocketSubsystem->RoleInstance_Client;
    return true;
}

bool FSocketRoleNone::HasPendingData(FSocketRedpointEOS &Socket, uint32 &PendingDataSize) const
{
    // Sockets with no role can never have pending data, as they don't have a valid FSocketEOSKey.
    return false;
}

bool FSocketRoleNone::SendTo(
    FSocketRedpointEOS &Socket,
    const uint8 *Data,
    int32 Count,
    int32 &BytesSent,
    const FInternetAddrRedpointEOS &Destination) const
{
    // Sockets with no role can not send packets - Connect or Listen should be called instead.
    return false;
}

bool FSocketRoleNone::RecvFrom(
    FSocketRedpointEOS &Socket,
    uint8 *Data,
    int32 BufferSize,
    int32 &BytesRead,
    FInternetAddr &Source,
    ESocketReceiveFlags::Type Flags) const
{
    // Sockets with no role can never have pending data, as they don't have a valid FSocketEOSKey.
    return false;
}

bool FSocketRoleNone::GetPeerAddress(class FSocketRedpointEOS &Socket, FInternetAddrRedpointEOS &OutAddr) const
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("GetPeerAddress called for socket with the None role."));
    return false;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
