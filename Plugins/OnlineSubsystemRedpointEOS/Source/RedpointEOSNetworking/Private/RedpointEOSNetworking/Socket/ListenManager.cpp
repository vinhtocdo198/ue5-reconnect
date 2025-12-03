// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/ListenManager.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/P2P/AcceptConnection.h"
#include "RedpointEOSAPI/P2P/CloseConnection.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketRoleListeningState.h"
#include "RedpointEOSNetworking/Socket/SocketSubsystemRedpointEOS.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(158820194, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;

void FListenManager::OnIncomingConnectionRequest(const FNotifyPeerConnectionRequest::Result &Info)
{
    FSocketListeningMapKey ListeningKey(GetAccountId(Info.LocalUserId), Info.SocketId);

    check(this->ListeningSockets.Contains(ListeningKey));

    auto LocalUserId = GetAccountId(Info.LocalUserId);
    auto RemoteUserId = GetAccountId(Info.RemoteUserId);

    bool bAccept = false;
    bool bAllNotBound = true;
    for (const auto &Socket : this->ListeningSockets[ListeningKey])
    {
        if (Socket->OnIncomingConnection.IsBound())
        {
            bAllNotBound = false;
            if (Socket->OnIncomingConnection.Execute(Socket, LocalUserId, RemoteUserId))
            {
                bAccept = true;
                break;
            }
        }
    }
    if (this->ListeningSockets[ListeningKey].Num() > 0 && bAllNotBound)
    {
        // If none of the sockets that are listening have an OnIncomingConnection delegate
        // registered, then we default to accept. This might be because the user of the socket
        // does not have access or is not using ISocketEOS, and is not filtering connections
        // at all.
        bAccept = true;
    }

    if (bAccept)
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("Accepting P2P connection from '%s' on socket ID '%s'"),
            *GetUserIdString(LocalUserId),
            *Info.SocketId);

        auto AcceptResult = FAcceptConnection::Execute(
            this->PlatformHandle,
            FAcceptConnection::Options{Info.LocalUserId, Info.RemoteUserId, Info.SocketId});
        if (AcceptResult != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Error,
                TEXT("Failed to accept connection, got status code %s"),
                *ConvertError(AcceptResult).ToLogString());
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("Rejecting P2P connection from '%s' on socket ID '%s'"),
            *GetUserIdString(LocalUserId),
            *Info.SocketId);

        // In issue #216, the EOS SDK does not correctly tell the client that their
        // connection was rejected (it doesn't raise the RemoteConnectionClosed event). This
        // means the client doesn't know that their pending connection was terminated.
        //
        // However, we can workaround this by accepting-then-closing the connection
        // on the server. This seems to be enough to get the client to properly notify the
        // plugin that the connection was closed.

        auto AcceptResult = FAcceptConnection::Execute(
            this->PlatformHandle,
            FAcceptConnection::Options{Info.LocalUserId, Info.RemoteUserId, Info.SocketId});
        if (AcceptResult != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Error,
                TEXT("Failed to temporarily accept connection prior to reject, got status code %s"),
                *ConvertError(AcceptResult).ToLogString());
        }

        // Now actually close/reject the connection.

        auto RejectResult = FCloseConnection::Execute(
            this->PlatformHandle,
            FCloseConnection::Options{Info.LocalUserId, Info.RemoteUserId, Info.SocketId});
        if (RejectResult != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Error,
                TEXT("Failed to reject connection, got status code %s"),
                *ConvertError(AcceptResult).ToLogString());
        }
    }
}

void FListenManager::OnRemoteConnectionClosed(const FNotifyPeerConnectionClosed::Result &Info)
{
    FSocketListeningMapKey ClosedListeningKey(GetAccountId(Info.LocalUserId), Info.SocketId);
    FString ClosedRemoteUserId = GetUserIdString(GetAccountId(Info.RemoteUserId));

    // Find all of the listening keys that might want to know about this connection closing.
    // @todo: This is tightly coupled to the internal state of sockets; we should probably abstract it out at some
    // point.
    for (const auto &ListeningKey : this->ListeningSockets)
    {
        if (ListeningKey.Key.ToString() == ClosedListeningKey.ToString())
        {
            for (const auto &ListeningSocket : ListeningKey.Value)
            {
                auto ListeningRoleState = StaticCastSharedRef<FSocketRoleListeningState>(ListeningSocket->RoleState);

                for (auto i = ListeningRoleState->RemoteSockets.Num() - 1; i >= 0; i--)
                {
                    auto RemoteSocket = ListeningRoleState->RemoteSockets[i];
                    if (auto RemoteSocketPinned = RemoteSocket.Pin())
                    {
                        auto InternetAddr = StaticCastSharedRef<FInternetAddrRedpointEOS>(
                            RemoteSocketPinned->SocketSubsystem->CreateInternetAddr());
                        RemoteSocketPinned->GetPeerAddress(*InternetAddr);
                        auto RemoteUserId = GetUserIdString(InternetAddr->GetUserId());
                        if (RemoteUserId == ClosedRemoteUserId)
                        {
                            UE_LOG(
                                LogRedpointEOSNetworking,
                                Verbose,
                                TEXT("Listening socket %s detected that remote socket %s was closed by the remote "
                                     "host; removing remote socket."),
                                *ListeningKey.Key.ToString(),
                                *RemoteUserId);

                            // Close the remote socket.
                            RemoteSocketPinned->bAlreadyClosedByRemoteHost = true;
                            RemoteSocketPinned->Close();
                        }
                    }
                }
            }
        }
    }

    UE_LOG(LogRedpointEOSNetworking, Verbose, TEXT("Remote connection closed!"));
}

FListenManager::FListenManager(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , ListeningSockets()
    , AcceptEvents()
    , ClosedEvents()
{
}

void FListenManager::RegisterEvents()
{
    this->PlatformHandle->OnNetworkStatusChanged().AddSP(this, &FListenManager::OnNetworkStatusChanged);
}

void FListenManager::OnNetworkStatusChanged(
    FPlatformInstance &Instance,
    EOS_ENetworkStatus OldNetworkStatus,
    EOS_ENetworkStatus NewNetworkStatus)
{
    if (OldNetworkStatus != EOS_ENetworkStatus::EOS_NS_Online && NewNetworkStatus == EOS_ENetworkStatus::EOS_NS_Online)
    {
        // We went from offline to online, we must restore events.
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("Restoring P2P connection and listening events after change in network status..."));
        this->AcceptEvents.Empty();
        this->ClosedEvents.Empty();
        for (const auto &KV : this->EventsToRestore)
        {
            this->AcceptEvents.Add(
                KV.Key,
                FNotifyPeerConnectionRequest::Register(
                    this->PlatformHandle,
                    FNotifyPeerConnectionRequest::Options{GetProductUserId(KV.Value.Get<0>()), KV.Value.Get<1>()},
                    FNotifyPeerConnectionRequest::EventDelegate::CreateSPLambda(
                        this,
                        [this](const FNotifyPeerConnectionRequest::Result &Info) {
                            this->OnIncomingConnectionRequest(Info);
                        })));
            this->ClosedEvents.Add(
                KV.Key,
                FNotifyPeerConnectionClosed::Register(
                    this->PlatformHandle,
                    FNotifyPeerConnectionClosed::Options{GetProductUserId(KV.Value.Get<0>()), KV.Value.Get<1>()},
                    FNotifyPeerConnectionClosed::EventDelegate::CreateSPLambda(
                        this,
                        [this](const FNotifyPeerConnectionClosed::Result &Info) {
                            this->OnRemoteConnectionClosed(Info);
                        })));
        }
    }
}

bool FListenManager::Add(FSocketRedpointEOS &InSocket, const FInternetAddrRedpointEOS &InLocalAddr)
{
    FSocketListeningMapKey ListeningKey(InLocalAddr.GetUserId(), InLocalAddr.GetSymmetricSocketId());

    if (this->ListeningSockets.Contains(ListeningKey))
    {
        this->ListeningSockets[ListeningKey].Add(InSocket.AsSharedRedpointEOS());
        return true;
    }

    // New socket to listen on; set up the events.

    TOptional<FSocketId> SocketId = InLocalAddr.GetSymmetricSocketId();

    this->AcceptEvents.Add(
        ListeningKey,
        FNotifyPeerConnectionRequest::Register(
            this->PlatformHandle,
            FNotifyPeerConnectionRequest::Options{GetProductUserId(InLocalAddr.GetUserId()), SocketId},
            FNotifyPeerConnectionRequest::EventDelegate::CreateSPLambda(
                this,
                [this](const FNotifyPeerConnectionRequest::Result &Info) {
                    this->OnIncomingConnectionRequest(Info);
                })));

    this->ClosedEvents.Add(
        ListeningKey,
        FNotifyPeerConnectionClosed::Register(
            this->PlatformHandle,
            FNotifyPeerConnectionClosed::Options{GetProductUserId(InLocalAddr.GetUserId()), SocketId},
            FNotifyPeerConnectionClosed::EventDelegate::CreateSPLambda(
                this,
                [this](const FNotifyPeerConnectionClosed::Result &Info) {
                    this->OnRemoteConnectionClosed(Info);
                })));

    this->EventsToRestore.Add(
        ListeningKey,
        TTuple<UE::Online::FAccountId, TOptional<FSocketId>>(InLocalAddr.GetUserId(), SocketId));

    this->ListeningSockets.Add(ListeningKey, TArray<TSharedRef<FSocketRedpointEOS>>());
    this->ListeningSockets[ListeningKey].Add(InSocket.AsSharedRedpointEOS());

    return true;
}

bool FListenManager::Remove(FSocketRedpointEOS &InSocket, const FInternetAddrRedpointEOS &InLocalAddr)
{
    FSocketListeningMapKey ListeningKey(InLocalAddr.GetUserId(), InLocalAddr.GetSymmetricSocketId());

    if (!this->ListeningSockets.Contains(ListeningKey))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FListenManager::Remove called for socket that is not registered with listen "
                 "manager"));
        return false;
    }

#if DO_CHECK
    auto OldNum = this->ListeningSockets[ListeningKey].Num();
#endif
    this->ListeningSockets[ListeningKey].Remove(InSocket.AsSharedRedpointEOS());
#if DO_CHECK
    check(this->ListeningSockets[ListeningKey].Num() == OldNum - 1);
#endif

    if (this->ListeningSockets[ListeningKey].Num() == 0)
    {
        this->ListeningSockets.Remove(ListeningKey);

        // This will also de-register the events.
        this->EventsToRestore.Remove(ListeningKey);
        this->AcceptEvents.Remove(ListeningKey);
        this->ClosedEvents.Remove(ListeningKey);
    }

    return true;
}

bool FListenManager::GetListeningSocketForNewInboundConnection(
    const FSocketConnectionMapKey &InLookupKey,
    TSharedPtr<FSocketRedpointEOS> &OutSocket)
{
    FSocketListeningMapKey ListeningKey(InLookupKey.GetLocalUserId(), InLookupKey.GetSymmetricSocketId());

    if (this->ListeningSockets.Contains(ListeningKey))
    {
        for (const auto &Socket : this->ListeningSockets[ListeningKey])
        {
            if (Socket->BindAddress->GetSymmetricChannel() == InLookupKey.GetSymmetricChannel())
            {
                OutSocket = Socket;
                return true;
            }
        }
    }

    return false;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
