// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAntiCheat/AntiCheatSystem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/NetDriver.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(2367027670, Redpoint::EOS::Networking::Socket, FSocketSubsystemRedpointEOS)
#endif
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1038774575,
    Redpoint::EOS::Networking::MultiIpResolve,
    FSocketSubsystemMultiIpResolve)

namespace REDPOINT_EOS_FILE_NS_ID(911548221, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::MultiIpResolve;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Networking::Socket;
#endif

class FNetDriverImpl : public TSharedFromThis<FNetDriverImpl>, public INetDriver
{
public:
    const TSoftObjectPtr<UWorld> World;
    const TSoftObjectPtr<URedpointEOSNetDriver> NetDriver;
    const FPlatformHandle PlatformHandle;
    const FPlatformOptionalRefCountedHandle PlatformOptionalRefCountedHandle;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TWeakPtr<FSocketRedpointEOS> Socket;
    bool IsDeferringToIpNetDriver;
#endif
    TOptional<UE::Online::FAccountId> RegisteredListeningAccount;
#if !UE_BUILD_SHIPPING
    const TSharedPtr<FSocketSubsystemMultiIpResolve> MultiIpResolveSubsystem;
#endif

    TArray<TSoftObjectPtr<URedpointEOSNetConnection>> PendingConnectionClose;

public:
    FNetDriverImpl(
        const TSoftObjectPtr<UWorld> &InWorld,
        const TSoftObjectPtr<URedpointEOSNetDriver> &InNetDriver,
        const FPlatformHandle &InPlatformHandle,
        const FPlatformOptionalRefCountedHandle &InPlatformOptionalRefCountedHandle);
    UE_NONCOPYABLE(FNetDriverImpl);
    virtual ~FNetDriverImpl() override = default;

private:
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    URedpointEOSNetConnection *CreateConnection();
#endif

    bool IsOwnedByBeacon();
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    bool CanPerformP2PNetworking(const TSharedPtr<FSocketSubsystemRedpointEOS> &InSocketSubsystem);
    bool CreateEOSSocket(
        const TSharedPtr<FSocketSubsystemRedpointEOS> &InSocketSubsystem,
        const FString &InDescription,
        const FURL &InURL,
        bool bListening,
        TSharedPtr<FSocketRedpointEOS> &OutSocket,
        UE::Online::FAccountId &OutBindingUserId);
#endif

    void SendAntiCheatData(
        const TSharedRef<FAntiCheatSession> &Session,
        const UE::Online::FAccountId &SourceUserId,
        const UE::Online::FAccountId &TargetUserId,
        const uint8 *Data,
        uint32_t Size);

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    bool Socket_OnIncomingConnection(
        const TSharedRef<FSocketRedpointEOS> &Socket,
        const UE::Online::FAccountId &LocalUser,
        const UE::Online::FAccountId &RemoteUser);
    void Socket_OnConnectionAccepted(
        const TSharedRef<FSocketRedpointEOS> &ListeningSocket,
        const TSharedRef<FSocketRedpointEOS> &AcceptedSocket,
        const UE::Online::FAccountId &LocalUser,
        const UE::Online::FAccountId &RemoteUser);
    void Socket_OnConnectionClosed(
        const TSharedRef<FSocketRedpointEOS> &ListeningSocket,
        const TSharedRef<FSocketRedpointEOS> &ClosedSocket,
        const FString &CloseSource);
#endif

    TOptional<UE::Online::FAccountId> GetFirstSignedInUser() const;

public:
    virtual void RequestConnectionClose(URedpointEOSNetConnection *InConnection) override;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual bool IsIpMode() const override;
    virtual TOptional<UE::Online::FAccountId> GetRegisteredListeningUser() const override;
    virtual TSharedPtr<FSocketRedpointEOS> GetRedpointEOSSocket() const override;
#endif

    virtual ENetDriverRole GetEOSRole() override;
    virtual TOptional<FPlatformHandle> GetPlatformHandle() override;

    bool IsAvailable() const;
    virtual bool IsNetResourceValid() override;
    virtual ISocketSubsystem *GetSocketSubsystem(URedpointEOSNetDriver *NetDriverPtr) override;

    virtual bool InitConnect(FNetworkNotify *InNotify, const FURL &ConnectURL, FString &Error) override;
    virtual bool InitListen(FNetworkNotify *InNotify, FURL &ListenURL, bool bReuseAddressAndPort, FString &Error)
        override;

    // We strip out the StatelessConnectionHandler from the packet handler stack for
    // P2P connections, since the P2P framework already manages the initial connection
    // (and thus, there's no point including any of the DDoS/stateless connection management
    // for P2P connections; it just makes the connection process more brittle).
    virtual void InitConnectionlessHandler() override;

    // Checks for incoming packets on the socket, and dispatches them to the connections.
    virtual void TickDispatch(float DeltaTime) override;

    // Flushes network packets. We close connections before calling Super::TickFlush to avoid
    // a crash.
    virtual void TickFlush(float DeltaTime) override;

    virtual void LowLevelSend(
        TSharedPtr<const FInternetAddr> Address,
        void *Data,
        int32 CountBits,
        FOutPacketTraits &Traits) override;
    virtual void LowLevelDestroy(URedpointEOSNetDriver *NetDriverPtr) override;
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(911548221, Redpoint::EOS::Networking, FNetDriverImpl)
}

REDPOINT_EOS_CODE_GUARD_END()