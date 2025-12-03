// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_SERVER_CODE
#include "RedpointEOSAPI/AntiCheatServer/NotifyClientActionRequired.h"
#include "RedpointEOSAPI/AntiCheatServer/NotifyClientAuthStatusChanged.h"
#include "RedpointEOSAPI/AntiCheatServer/NotifyMessageToClient.h"
#include "RedpointEOSAntiCheat/AntiCheatSystem.h"
#include "RedpointEOSAntiCheat/DedicatedServerAntiCheatSession.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace REDPOINT_EOS_FILE_NS_ID(3183064733, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::AntiCheatServer;
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Utils;

class FDedicatedServerAntiCheatSystemImpl : public IAntiCheatSystem,
                                            public IHasEventRegistration,
                                            public TSharedFromThis<FDedicatedServerAntiCheatSystemImpl>
{
private:
    FPlatformHandle PlatformHandle;
    TSharedPtr<FDedicatedServerAntiCheatSession> CurrentSession;
    FEventHandlePtr NotifyMessageToClient;
    FEventHandlePtr NotifyClientActionRequired;
    FEventHandlePtr NotifyClientAuthStatusChanged;

    void EnsureSessionIsCurrent(FAntiCheatSession &Session, const TCHAR *FunctionName) const;

public:
    FDedicatedServerAntiCheatSystemImpl(const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FDedicatedServerAntiCheatSystemImpl);
    virtual ~FDedicatedServerAntiCheatSystemImpl() override = default;

    virtual void RegisterEvents() override;

private:
    void OnNotifyMessageToClient(const FNotifyMessageToClient::Result &Result);
    void OnNotifyClientActionRequired(const FNotifyClientActionRequired::Result &Result);
    void OnNotifyClientAuthStatusChanged(const FNotifyClientAuthStatusChanged::Result &Result);

public:
    virtual bool ShouldGameExitOnStartup() const override;

    virtual EAntiCheatImplementationType GetImplementationType() const override;

    virtual bool ShouldRemoteSkipPeerRegistration() const override;

    virtual TSharedPtr<FAntiCheatSession> CreateSession(
        bool bIsServer,
        UE::Online::FAccountId HostUserId,
        bool bIsDedicatedServerSession,
        TOptional<UE::Online::FAccountId> ListenServerUserId,
        FString ServerConnectionUrlOnClient) override;
    virtual bool DestroySession(FAntiCheatSession &Session) override;

    virtual bool RegisterPlayer(
        FAntiCheatSession &Session,
        UE::Online::FAccountId UserId,
        EOS_EAntiCheatCommonClientType ClientType,
        EOS_EAntiCheatCommonClientPlatform ClientPlatform) override;
    virtual bool UnregisterPlayer(FAntiCheatSession &Session, UE::Online::FAccountId UserId) override;

    virtual bool ReceiveNetworkMessage(
        FAntiCheatSession &Session,
        UE::Online::FAccountId SourceUserId,
        UE::Online::FAccountId TargetUserId,
        const uint8 *Data,
        uint32_t Size) override;
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(3183064733, Redpoint::EOS::AntiCheat, FDedicatedServerAntiCheatSystemImpl)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()