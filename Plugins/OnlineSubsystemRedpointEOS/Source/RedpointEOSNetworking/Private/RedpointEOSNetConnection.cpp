// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetConnection.h"

#include "EncryptionComponent.h"
#include "Misc/CommandLine.h"
#include "Net/DataChannel.h"
#include "Net/NetConnectionFaultRecovery.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSNetworking/EmptyHandshakeHandlerComponent.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/NetConnectionImpl.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

#define REDPOINT_EOS_CHECK_IMPL_SET()                                                                                  \
    checkf(                                                                                                            \
        this->Impl.IsValid(),                                                                                          \
        TEXT("%s: Expected NetConnectionImpl to have been assigned already!"),                                         \
        ANSI_TO_TCHAR(__FUNCTION__))

template <typename Tag, typename Tag::type M> struct PrivateAccessor
{
    friend typename Tag::type get(Tag)
    {
        return M;
    }
};

/**
 * Workaround to gain access to private PacketHandler fields
 */
struct PacketHandler_EncryptionComponent
{
    typedef TSharedPtr<FEncryptionComponent> PacketHandler::*type;
    friend type get(PacketHandler_EncryptionComponent);
};
struct PacketHandler_HandlerComponents
{
    typedef TArray<TSharedPtr<HandlerComponent>> PacketHandler::*type;
    friend type get(PacketHandler_HandlerComponents);
};

template struct PrivateAccessor<PacketHandler_EncryptionComponent, &PacketHandler::EncryptionComponent>;
template struct PrivateAccessor<PacketHandler_HandlerComponents, &PacketHandler::HandlerComponents>;

/**
 * End of workaround to gain access to private PacketHandler fields
 */

void URedpointEOSNetConnection::InitHandler()
{
    using namespace UE::Net;
    using namespace ::Redpoint::EOS::Networking;

    check(!Handler.IsValid());

#if !UE_BUILD_SHIPPING
    if (!FParse::Param(FCommandLine::Get(), TEXT("NoPacketHandler")))
#endif
    {
        Handler = MakeUnique<PacketHandler>();

        if (Handler.IsValid())
        {
            UE::Handler::Mode Mode =
                Driver->ServerConnection != nullptr ? UE::Handler::Mode::Client : UE::Handler::Mode::Server;

            UE_LOG(
                LogRedpointEOSNetworking,
                VeryVerbose,
                TEXT("Set Handler mode: %s"),
                Mode == UE::Handler::Mode::Client ? TEXT("Client") : TEXT("Server"));

            FPacketHandlerNotifyAddHandler NotifyAddHandler;

            NotifyAddHandler.BindLambda([this](TSharedPtr<HandlerComponent> &NewHandler) {
                if (NewHandler.IsValid())
                {
                    NewHandler->InitFaultRecovery(GetFaultRecovery());
                }
            });

            UE_LOG(LogRedpointEOSNetworking, VeryVerbose, TEXT("Adding FEmptyHandshakeHandlerComponent..."));
            /**
             * At least 1 handler must require a handshake in order for Unreal Engine's netcode to function properly, so
             * we add this placeholder handshake handler component which sets itself as ready as soon as
             * NotifyHandshakeBegin() is called.
             */
            TSharedPtr<HandlerComponent> EmptyHandshakeHandler = MakeShared<FEmptyHandshakeHandlerComponent>();
            Handler->AddHandler(EmptyHandshakeHandler, true);

            if (Mode == UE::Handler::Mode::Server)
            {
                UE_LOG(
                    LogRedpointEOSNetworking,
                    VeryVerbose,
                    TEXT("[Server] Calling EmptyHandshakeHandler->NotifyHandshakeBegin()..."));
                // NotifyHandshakeBegin does not get called on the server, so we need to kick it off here instead
                StaticCastSharedPtr<FEmptyHandshakeHandlerComponent>(EmptyHandshakeHandler)->NotifyHandshakeBegin();
            }

            Handler->InitializeDelegates(
                FPacketHandlerLowLevelSendTraits::CreateUObject(this, &UNetConnection::LowLevelSend),
                MoveTemp(NotifyAddHandler));

            UE_LOG(LogRedpointEOSNetworking, VeryVerbose, TEXT("[Server] Initializing Handler..."));
            Handler->Initialize(
                Mode,
                static_cast<uint32>(MaxPacket * 8),
                false,
                nullptr,
                nullptr,
                Driver->GetNetDriverDefinition());

            /**
             * Handler->Initialize adds an AES encryption handler component which is not necessary in peer to peer
             * networking (P2P traffic is already encrypted over WebRTC). Remove the AES handler as it will not be used.
             *
             * Also Handler->EncryptionComponent and Handler->HandlerComponents are private fields so we need to get
             * slightly creative so that we can access them here without modifying game engine code.
             */
            UE_LOG(
                LogRedpointEOSNetworking,
                VeryVerbose,
                TEXT("[Server] Removing encryption component from Handler..."));
            TArray<TSharedPtr<HandlerComponent>> &HandlerComponents =
                Handler.Get()->*get(PacketHandler_HandlerComponents());
            TSharedPtr<FEncryptionComponent> &EncryptionComponent =
                Handler.Get()->*get(PacketHandler_EncryptionComponent());
            HandlerComponents.Remove(StaticCastSharedPtr<HandlerComponent>(EncryptionComponent));
            EncryptionComponent = nullptr;

            /**
             * Handler components can now be initialized, now that we've removed the AES handler component
             */
            UE_LOG(LogRedpointEOSNetworking, VeryVerbose, TEXT("[Server] Initializing Handler components..."));
            Handler->InitializeComponents();
        }
    }
}

void URedpointEOSNetConnection::InitBase(
    UNetDriver *InDriver,
    FSocket *InSocket,
    const FURL &InURL,
    EConnectionState InState,
    int32 InMaxPacket,
    int32 InPacketOverhead)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    using namespace ::Redpoint::EOS::Networking::Socket;
#endif

    // According to USteamSocketsNetConnection, UNetConnection can assert if this value is greater
    // than 1024 (though I can't find out where it would do so).
    static const int32 MAX_PACKET = EOS_P2P_MAX_PACKET_SIZE > 1024 ? 1024 : EOS_P2P_MAX_PACKET_SIZE;

    UNetConnection::InitBase(
        InDriver,
        InSocket,
        InURL,
        InState,
        ((InMaxPacket == 0) ? MAX_PACKET : InMaxPacket),
        ((InPacketOverhead == 0) ? 1 : InPacketOverhead));

    REDPOINT_EOS_CHECK_IMPL_SET();

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    this->Impl->SetSocket(static_cast<FSocketRedpointEOS *>(InSocket)->AsSharedRedpointEOS());
#endif
}

void URedpointEOSNetConnection::InitRemoteConnection(
    UNetDriver *InDriver,
    FSocket *InSocket,
    const FURL &InURL,
    const FInternetAddr &InRemoteAddr,
    EConnectionState InState,
    int32 InMaxPacket,
    int32 InPacketOverhead)
{
    this->InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

    this->RemoteAddr = InRemoteAddr.Clone();

    this->InitSendBuffer();
    this->SetClientLoginState(EClientLoginState::LoggingIn);
    this->SetExpectedClientLoginMsgType(NMT_Hello);
}

void URedpointEOSNetConnection::InitLocalConnection(
    UNetDriver *InDriver,
    FSocket *InSocket,
    const FURL &InURL,
    EConnectionState InState,
    int32 InMaxPacket,
    int32 InPacketOverhead)
{
    this->InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

    // Get the remote address (the one we are connecting to) from the InURL.
    this->RemoteAddr =
        InDriver->GetSocketSubsystem()->GetAddressFromString(FString::Printf(TEXT("%s:%d"), *InURL.Host, InURL.Port));

    this->InitSendBuffer();
}

void URedpointEOSNetConnection::LowLevelSend(void *Data, int32 CountBits, FOutPacketTraits &Traits)
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    this->Impl->LowLevelSend(Data, CountBits, Traits);
}

FString URedpointEOSNetConnection::LowLevelGetRemoteAddress(bool bAppendPort)
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    return this->Impl->LowLevelGetRemoteAddress(bAppendPort);
}

FString URedpointEOSNetConnection::LowLevelDescribe()
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    return this->Impl->LowLevelDescribe();
}

TSharedRef<::Redpoint::EOS::Networking::FNetConnectionImpl> URedpointEOSNetConnection::GetImpl() const
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    return this->Impl.ToSharedRef();
}

#undef REDPOINT_EOS_CHECK_IMPL_SET

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()