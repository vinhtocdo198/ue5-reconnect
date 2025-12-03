// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/EventHandle.h"
#include "RedpointEOSAPI/P2P/NotifyPeerConnectionClosed.h"
#include "RedpointEOSAPI/P2P/NotifyPeerConnectionRequest.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#include "RedpointEOSNetworking/Socket/SocketListeningMap.h"
#include "RedpointEOSNetworking/Socket/SocketListeningMapKey.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(158820194, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::P2P;
using namespace ::Redpoint::EOS::Networking::Socket;
using namespace ::Redpoint::EOS::Core::Utils;

class FListenManager : public TSharedFromThis<FListenManager>, public IHasEventRegistration
{
private:
    FPlatformHandle PlatformHandle;

    TSocketListeningMap<TArray<TSharedRef<FSocketRedpointEOS>>> ListeningSockets;
    TSocketListeningMap<FEventHandlePtr> AcceptEvents;
    TSocketListeningMap<FEventHandlePtr> ClosedEvents;
    TSocketListeningMap<TTuple<UE::Online::FAccountId, TOptional<FSocketId>>> EventsToRestore;

    void OnIncomingConnectionRequest(const FNotifyPeerConnectionRequest::Result &Info);
    void OnRemoteConnectionClosed(const FNotifyPeerConnectionClosed::Result &Info);

public:
    FListenManager(const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FListenManager);
    virtual ~FListenManager() override = default;

    virtual void RegisterEvents() override;

private:
    void OnNetworkStatusChanged(
        FPlatformInstance &Instance,
        EOS_ENetworkStatus OldNetworkStatus,
        EOS_ENetworkStatus NewNetworkStatus);

public:
    bool Add(FSocketRedpointEOS &InSocket, const FInternetAddrRedpointEOS &InLocalAddr);
    bool Remove(FSocketRedpointEOS &InSocket, const FInternetAddrRedpointEOS &InLocalAddr);

    bool GetListeningSocketForNewInboundConnection(
        const FSocketConnectionMapKey &InLookupKey,
        TSharedPtr<FSocketRedpointEOS> &OutSocket);
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(158820194, Redpoint::EOS::Networking::Socket, FListenManager)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()