// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Engine/EngineBaseTypes.h"
#include "RedpointEOSAPI/EventHandle.h"
#include "RedpointEOSAPI/P2P/SocketId.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Utils/RegulatedTicker.h"
#include "RedpointEOSNetworking/Socket/SocketConnectionMap.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOSMemoryId.h"
#include "SocketSubsystem.h"
#include "Tickable.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4168139954, Redpoint::EOS::Networking::Socket, FSocketRoleNone)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3448274137, Redpoint::EOS::Networking::Socket, FSocketRoleClient)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(1773620347, Redpoint::EOS::Networking::Socket, FSocketRoleListening)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(767262975, Redpoint::EOS::Networking::Socket, FSocketRoleRemote)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(158820194, Redpoint::EOS::Networking::Socket, FListenManager)

namespace REDPOINT_EOS_FILE_NS_ID(2367027670, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Networking::Socket;

/**
 * @note: Although this implements ISocketSubsystem, it's not registered as a normal socket subsystem. Instead it's
 * registered as a system on the platform handle because the socket subsystems are unique per EOS SDK platform instance.
 */
class FSocketSubsystemRedpointEOS : public ISocketSubsystem,
                                    public IPlatformInstanceSystem,
                                    public TSharedFromThis<FSocketSubsystemRedpointEOS>
{
public:
    static FName GetSystemName();
    static TSharedRef<FSocketSubsystemRedpointEOS> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

private:
    void OnShutdown(FPlatformInstance &Instance);

public:
    /** The EOS platform handle. */
    FPlatformHandle PlatformHandle;

    /** If true, Shutdown has been called. */
    bool bIsShutdown;

    /** If true, the subsystem will not receive packets. */
    bool bReceiveSuspended;

    /**
     * Reference to the online subsystem. This is valid until Shutdown is called; we must clear it so that the OSS
     * IsUnique checks pass during shutdown.
     */
    TSharedRef<IConfig> Config;
    FDelegateHandle ReceivePacketsDelegateHandle;

    /** A map that tracks all sockets allocated in memory. */
    TMap<FSocketRedpointEOSMemoryId, TSharedPtr<FSocketRedpointEOS>> HeldSockets;
    static FSocketRedpointEOSMemoryId NextSocketId;

    /**
     * A map that tracks sockets for the purposes of routing received packets. A socket is only added to this map once
     * either Bind or SentTo has been called.
     */
    TSocketConnectionMap<TWeakPtr<FSocketRedpointEOS>> AssignedSockets;

    // Pre-allocated role instances.
    TSharedRef<FSocketRoleNone> RoleInstance_None;
    TSharedRef<FSocketRoleClient> RoleInstance_Client;
    TSharedRef<FSocketRoleListening> RoleInstance_Listening;
    TSharedRef<FSocketRoleRemote> RoleInstance_Remote;

    // The listen manager, which holds references to listening sockets and
    // routes the IncomingConnection and RemoteConnectionClosed events correctly.
    TSharedRef<FListenManager> ListenManager;

    // The next reset ID for an outbound client connection.
    uint32_t NextResetId;

    // Returns the next reset ID and increments it.
    uint32_t GetResetIdForOutboundConnection()
    {
        return this->NextResetId++;
    }

    // Tracks pending reset IDs; that is, when we receive a reset ID for a socket that's not yet open, we don't yet have
    // the socket to check / assign it to. This holds a list of reset IDs we have received from remote hosts so that
    // when ::Accept actually happens we can load it in.
    TMap<FString, uint32_t> PendingResetIds;

    // The event handler for when the incoming packet queue is full.
    FEventHandlePtr IncomingPacketQueueFull;

public:
    FSocketSubsystemRedpointEOS(const FPlatformHandle &InPlatform, TSharedRef<Redpoint::EOS::Config::IConfig> InConfig);
    UE_NONCOPYABLE(FSocketSubsystemRedpointEOS);
    virtual ~FSocketSubsystemRedpointEOS() override = default;

    /**
     * Returns the product user ID to use when binding P2P addresses.
     */
    TOptional<UE::Online::FAccountId> GetBindingProductUserId_P2POnly() const;

    FSocketId GetSocketName(bool bListening, FURL InURL) const;

    virtual bool Init(FString &Error) override;
    virtual void Shutdown() override;

    bool Tick_ReceiveP2PPackets(float DeltaSeconds);

    virtual FSocket *CreateSocket(const FName &SocketType, const FString &SocketDescription, const FName &ProtocolName)
        override;
    virtual void DestroySocket(FSocket *Socket) override;

    virtual FAddressInfoResult GetAddressInfo(
        const TCHAR *HostName,
        const TCHAR *ServiceName = nullptr,
        EAddressInfoFlags QueryFlags = EAddressInfoFlags::Default,
        const FName ProtocolTypeName = NAME_None,
        ESocketType SocketType = ESocketType::SOCKTYPE_Unknown) override;
    virtual TSharedPtr<FInternetAddr> GetAddressFromString(const FString &InAddress) override;

    virtual bool RequiresChatDataBeSeparate() override;
    virtual bool RequiresEncryptedPackets() override;
    virtual bool GetHostName(FString &HostName) override;

    virtual TSharedRef<FInternetAddr> CreateInternetAddr() override;
    virtual bool HasNetworkDevice() override;

    virtual const TCHAR *GetSocketAPIName() const override;
    virtual ESocketErrors GetLastErrorCode() override;
    virtual ESocketErrors TranslateErrorCode(int32 Code) override;

    virtual bool IsSocketWaitSupported() const override;

    void SuspendReceive();
    void ResumeReceive();
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(2367027670, Redpoint::EOS::Networking::Socket, FSocketSubsystemRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
