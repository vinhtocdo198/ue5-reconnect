// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/AntiCheatClient/NotifyClientIntegrityViolated.h"
#include "RedpointEOSAPI/AntiCheatClient/NotifyMessageToPeer.h"
#include "RedpointEOSAPI/AntiCheatClient/NotifyMessageToServer.h"
#include "RedpointEOSAPI/AntiCheatClient/NotifyPeerActionRequired.h"
#include "RedpointEOSAPI/AntiCheatClient/NotifyPeerAuthStatusChanged.h"
#include "RedpointEOSAntiCheat/AntiCheatSystem.h"
#include "RedpointEOSAntiCheat/GameAntiCheatSession.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace REDPOINT_EOS_FILE_NS_ID(1049104577, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::AntiCheatClient;
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Utils;

class FGameAntiCheatSystemImpl : public IAntiCheatSystem,
                                 public IHasEventRegistration,
                                 public TSharedFromThis<FGameAntiCheatSystemImpl>
{
private:
    FPlatformHandle PlatformHandle;
    bool bHasTrustedClientPrivateKey;
    bool bSuccessfullyInitialized;
    TSharedPtr<FGameAntiCheatSession> CurrentSession;
    FEventHandlePtr NotifyMessageToServer;
    FEventHandlePtr NotifyMessageToPeer;
    FEventHandlePtr NotifyClientIntegrityViolated;
    FEventHandlePtr NotifyPeerActionRequired;
    FEventHandlePtr NotifyPeerAuthStatusChanged;

    void EnsureSessionIsCurrent(FAntiCheatSession &Session, const TCHAR *FunctionName) const;

public:
    FGameAntiCheatSystemImpl(const FPlatformHandle &InPlatformHandle, bool bInHasTrustedClientPrivateKey);
    UE_NONCOPYABLE(FGameAntiCheatSystemImpl);
    virtual ~FGameAntiCheatSystemImpl() override = default;

    virtual void RegisterEvents() override;

private:
    void OnNotifyMessageToServer(const FNotifyMessageToServer::Result &Result);
    void OnNotifyMessageToPeer(const FNotifyMessageToPeer::Result &Result);
    void OnNotifyClientIntegrityViolated(const FNotifyClientIntegrityViolated::Result &Result);
    void OnNotifyPeerActionRequired(const FNotifyPeerActionRequired::Result &Result);
    void OnNotifyPeerAuthStatusChanged(const FNotifyPeerAuthStatusChanged::Result &Result);

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
REDPOINT_EOS_FILE_NS_EXPORT(1049104577, Redpoint::EOS::AntiCheat, FGameAntiCheatSystemImpl)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()