// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAntiCheat/AntiCheatImplementationType.h"
#include "RedpointEOSAntiCheat/AntiCheatSession.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(353349712, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::AntiCheat;

class REDPOINTEOSANTICHEAT_API IAntiCheatSystem : public IPlatformInstanceSystem
{
public:
    typedef TDelegate<void(
        const TSharedRef<FAntiCheatSession> &Session,
        const UE::Online::FAccountId &SourceUserId,
        const UE::Online::FAccountId &TargetUserId,
        const uint8 *Data,
        uint32_t Size)>
        FOnAntiCheatSendNetworkMessage;

    typedef TMulticastDelegate<
        void(UE::Online::FAccountId TargetUserId, EOS_EAntiCheatCommonClientAuthStatus NewAuthStatus)>
        FOnAntiCheatPlayerAuthStatusChanged;

    typedef TMulticastDelegate<void(
        UE::Online::FAccountId TargetUserId,
        EOS_EAntiCheatCommonClientAction ClientAction,
        EOS_EAntiCheatCommonClientActionReason ActionReasonCode,
        const FString &ActionReasonDetailsString)>
        FOnAntiCheatPlayerActionRequired;

private:
    FOnAntiCheatSendNetworkMessage OnSendNetworkMessageDelegate;
    FOnAntiCheatPlayerAuthStatusChanged OnPlayerAuthStatusChangedDelegate;
    FOnAntiCheatPlayerActionRequired OnPlayerActionRequiredDelegate;

public:
    static FName GetSystemName();
    static TSharedRef<IAntiCheatSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IAntiCheatSystem() = default;
    UE_NONCOPYABLE(IAntiCheatSystem);
    virtual ~IAntiCheatSystem() override = default;

    virtual bool ShouldGameExitOnStartup() const = 0;

    virtual EAntiCheatImplementationType GetImplementationType() const = 0;

    /**
     * If true and this is a trusted client, remote servers and peers are instructed
     * to not register this client at all, rather than using EOS_ACCCT_UnprotectedClient.
     * This is necessary for systems where EOS_HAntiCheatClient is null when running
     * without EAC, instead of where EOS_HAntiCheatClient is provided as an implementation
     * that can handle network messages.
     */
    virtual bool ShouldRemoteSkipPeerRegistration() const = 0;

    virtual TSharedPtr<FAntiCheatSession> CreateSession(
        bool bIsServer,
        UE::Online::FAccountId HostUserId,
        bool bIsDedicatedServerSession,
        TOptional<UE::Online::FAccountId> ListenServerUserId,
        FString ServerConnectionUrlOnClient) = 0;
    virtual bool DestroySession(FAntiCheatSession &Session) = 0;

    virtual bool RegisterPlayer(
        FAntiCheatSession &Session,
        UE::Online::FAccountId UserId,
        EOS_EAntiCheatCommonClientType ClientType,
        EOS_EAntiCheatCommonClientPlatform ClientPlatform) = 0;
    virtual bool UnregisterPlayer(FAntiCheatSession &Session, UE::Online::FAccountId UserId) = 0;

    virtual bool ReceiveNetworkMessage(
        FAntiCheatSession &Session,
        UE::Online::FAccountId SourceUserId,
        UE::Online::FAccountId TargetUserId,
        const uint8 *Data,
        uint32_t Size) = 0;

    /** Called by the IAntiCheat implementation when a network message needs to be sent. */
    virtual FOnAntiCheatSendNetworkMessage &OnSendNetworkMessage()
    {
        return this->OnSendNetworkMessageDelegate;
    }

    /** Called by the IAntiCheat implementation when a player's auth status has been verified. */
    virtual FOnAntiCheatPlayerAuthStatusChanged &OnPlayerAuthStatusChanged()
    {
        return this->OnPlayerAuthStatusChangedDelegate;
    }

    /** Called by the IAntiCheat implementation when a player needs to be kicked. */
    virtual FOnAntiCheatPlayerActionRequired &OnPlayerActionRequired()
    {
        return this->OnPlayerActionRequiredDelegate;
    }
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(353349712, Redpoint::EOS::AntiCheat, IAntiCheatSystem)
}

REDPOINT_EOS_CODE_GUARD_END()