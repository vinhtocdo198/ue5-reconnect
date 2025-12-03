// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Containers/Queue.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/RawPacket.h"
#include "RedpointEOSNetworking/Socket/SocketConnectionMapKey.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOSMemoryId.h"
#include "Sockets.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(1057720780, Redpoint::EOS::Networking::Socket, ISocketRole)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(698769264, Redpoint::EOS::Networking::Socket, ISocketRoleState)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(2367027670, Redpoint::EOS::Networking::Socket, FSocketSubsystemRedpointEOS)

namespace REDPOINT_EOS_FILE_NS_ID(4230421139, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Networking::Socket;

class FSocketRedpointEOS;

typedef TDelegate<bool /* Accept */ (
    const TSharedRef<FSocketRedpointEOS> &Socket,
    const UE::Online::FAccountId &LocalUser,
    const UE::Online::FAccountId &RemoteUser)>
    FSocketRedpointEOSOnIncomingConnection;

typedef TDelegate<void(
    const TSharedRef<FSocketRedpointEOS> &ListeningSocket,
    const TSharedRef<FSocketRedpointEOS> &AcceptedSocket,
    const UE::Online::FAccountId &LocalUser,
    const UE::Online::FAccountId &RemoteUser)>
    FSocketRedpointEOSOnConnectionAccepted;

typedef TDelegate<void(
    const TSharedRef<FSocketRedpointEOS> &ListeningSocket,
    const TSharedRef<FSocketRedpointEOS> &ClosedSocket,
    const FString &CloseSource)>
    FSocketRedpointEOSOnConnectionClosed;

class FSocketRedpointEOS : public FSocket
#if !REDPOINT_EOS_UE_5_6_OR_LATER
    ,
                           public TSharedFromThis<FSocketRedpointEOS>
#endif
{
public:
    /**
     * This is set to true when a socket starts closing, so that when Close() is entered a second time due to the
     * connection closing, it won't re-trigger the close logic.
     */
    bool bClosing;

    /**
     * This is set to true by FSocketRedpointEOS once Close() has been called on a socket, and is used
     * by UEOSNetConnection to detect when the socket has gone away and the connection should be closed.
     * This prevents UEOSNetConnection from continuing to try and use sockets that have been intentionally
     * closed by the remote endpoint (e.g. if a server rejects the connection).
     */
    bool bClosed;

    /**
     * This is set to true by FListenManager::OnRemoteConnectionClosed when it is notified that a remote connection has
     * already been closed, and thus CloseConnection must not be called again.
     */
    bool bAlreadyClosedByRemoteHost;

    /**
     * The current role for this socket - the role is the actual implementation of behaviour for the socket.
     */
    TSharedRef<ISocketRole> Role;

    /**
     * Custom state that the role implementation uses between requests. When the role changes, the role state will be
     * replaced.
     */
    TSharedRef<ISocketRoleState> RoleState;

    /** Reference to the EOS P2P interface. */
    FPlatformHandle PlatformHandle;

    /** The local address we are bound on. The user ID of this address is the sender of packets. */
    TSharedPtr<FInternetAddrRedpointEOS> BindAddress;

    /**
     * The memory ID of this socket; this ID is used to ensure that sockets are freed correctly when they are no longer
     * used.
     */
    FSocketRedpointEOSMemoryId SocketMemoryId;

    /**
     * The unique key used to identify this socket when the socket subsystem needs to route received packets to it. This
     * is a pointer because it doesn't get set to a valid value until after an operation is performed on the socket.
     */
    TSharedPtr<FSocketConnectionMapKey> SocketKey;

    /**
     * When you are updating the socket key, you must call this function so the socket can register itself with the
     * socket subsystem correctly.
     */
    void UpdateSocketKey(const FSocketConnectionMapKey &InSocketKey);

    /**
     * Used when removing the current socket key from the socket subsystem. Automatically calls EOS_P2P_CloseConnection
     * if this was the last socket that could receive packets from the remote connection.
     */
    void RemoveSocketKeyIfSet();

    /**
     * A list of received packets that the socket subsystem has received and assigned to this socket, but for which
     * the socket RecvFrom function has not yet been called to actually receive them.
     */
    TQueue<TSharedPtr<FRawPacket>> ReceivedPacketsQueue;

    /**
     * The number of packets in the received queue, since TQueue does not have a Num() function.
     */
    int32 ReceivedPacketsQueueCount;

    /**
     * Note: We don't use this field for anything, except to ensure that the socket subsystem doesn't get destroyed
     * before the socket is. This is important because the socket registers events with EOS, and we need to ensure that
     * the online subsystem doesn't clean up the EOS_HPlatform handle while there are still events registered.
     */
    TSharedPtr<FSocketSubsystemRedpointEOS> SocketSubsystem;

    /**
     * When a new connection is arriving that can potentially send data to this socket, this event is called
     * to evaluate whether or not the connection should be accepted. Only one socket needs to return true
     * from this function in order for a connection to be accepted (and connections are evaluated at the socket ID
     * level, not the channel level, so it's possible for this handler to return false or not be called at all).
     */
    FSocketRedpointEOSOnIncomingConnection OnIncomingConnection;

    /**
     * When a new remote socket has been accepted by this socket, this event is called so the owner of the listening
     * socket (this socket) can appropriately update it state to reflect the new connection.
     */
    FSocketRedpointEOSOnConnectionAccepted OnConnectionAccepted;

    /**
     * When the remote host has closed the connection, this event is called.
     */
    FSocketRedpointEOSOnConnectionClosed OnConnectionClosed;

    /**
     * Off by default. If turned on, all send operations will use reliable ordered instead of unreliable unordered.
     */
    bool bEnableReliableSending;

    FSocketRedpointEOS(
        const TSharedRef<FSocketSubsystemRedpointEOS> &InSocketSubsystem,
        FPlatformHandle InPlatformHandle,
        FSocketRedpointEOSMemoryId InSocketMemoryId,
        const FString &InSocketDescription);
    UE_NONCOPYABLE(FSocketRedpointEOS);
    virtual ~FSocketRedpointEOS() override;

    int GetSocketMemoryId() const
    {
        return this->SocketMemoryId;
    }

    virtual bool Shutdown(ESocketShutdownMode Mode) override;
    virtual bool Close() override;
    virtual bool Bind(const FInternetAddr &Addr) override;
    virtual bool Connect(const FInternetAddr &Addr) override;
    virtual bool Listen(int32 MaxBacklog) override;
    virtual bool WaitForPendingConnection(bool &bHasPendingConnection, const FTimespan &WaitTime) override;
    virtual bool HasPendingData(uint32 &PendingDataSize) override;
    virtual FSocket *Accept(const FString &InSocketDescription) override;
    virtual FSocket *Accept(FInternetAddr &InRemoteAddr, const FString &InSocketDescription) override;
    virtual bool SendTo(const uint8 *Data, int32 Count, int32 &BytesSent, const FInternetAddr &Destination) override;
    virtual bool Send(const uint8 *Data, int32 Count, int32 &BytesSent) override;
    virtual bool RecvFrom(
        uint8 *Data,
        int32 BufferSize,
        int32 &BytesRead,
        FInternetAddr &Source,
        ESocketReceiveFlags::Type Flags = ESocketReceiveFlags::None) override;
    virtual bool Recv(
        uint8 *Data,
        int32 BufferSize,
        int32 &BytesRead,
        ESocketReceiveFlags::Type Flags = ESocketReceiveFlags::None) override;
    virtual bool RecvMulti(FRecvMulti &MultiData, ESocketReceiveFlags::Type Flags = ESocketReceiveFlags::None) override;
    virtual bool Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime) override;
    virtual ESocketConnectionState GetConnectionState() override;
    virtual void GetAddress(FInternetAddr &OutAddr) override;
    virtual bool GetPeerAddress(FInternetAddr &OutAddr) override;
    virtual bool SetNonBlocking(bool bIsNonBlocking = true) override;
    virtual bool SetBroadcast(bool bAllowBroadcast = true) override;
    virtual bool SetNoDelay(bool bIsNoDelay = true) override;
    virtual bool JoinMulticastGroup(const FInternetAddr &GroupAddress) override;
    virtual bool JoinMulticastGroup(const FInternetAddr &GroupAddress, const FInternetAddr &InterfaceAddress) override;
    virtual bool LeaveMulticastGroup(const FInternetAddr &GroupAddress) override;
    virtual bool LeaveMulticastGroup(const FInternetAddr &GroupAddress, const FInternetAddr &InterfaceAddress) override;
    virtual bool SetMulticastLoopback(bool bLoopback) override;
    virtual bool SetMulticastTtl(uint8 TimeToLive) override;
    virtual bool SetMulticastInterface(const FInternetAddr &InterfaceAddress) override;
    virtual bool SetReuseAddr(bool bAllowReuse = true) override;
    virtual bool SetLinger(bool bShouldLinger = true, int32 Timeout = 0) override;
    virtual bool SetRecvErr(bool bUseErrorQueue = true) override;
    virtual bool SetSendBufferSize(int32 Size, int32 &NewSize) override;
    virtual bool SetReceiveBufferSize(int32 Size, int32 &NewSize) override;
    virtual int32 GetPortNo() override;

    TSharedRef<FSocketRedpointEOS> AsSharedRedpointEOS()
    {
#if REDPOINT_EOS_UE_5_6_OR_LATER
        return StaticCastSharedRef<FSocketRedpointEOS>(this->AsShared());
#else
        return this->AsShared();
#endif
    }

    TSharedRef<const FSocketRedpointEOS> AsSharedRedpointEOS() const
    {
#if REDPOINT_EOS_UE_5_6_OR_LATER
        return StaticCastSharedRef<const FSocketRedpointEOS>(this->AsShared());
#else
        return this->AsShared();
#endif
    }
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOSOnIncomingConnection)
REDPOINT_EOS_FILE_NS_EXPORT(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOSOnConnectionAccepted)
REDPOINT_EOS_FILE_NS_EXPORT(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOSOnConnectionClosed)
REDPOINT_EOS_FILE_NS_EXPORT(4230421139, Redpoint::EOS::Networking::Socket, FSocketRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
