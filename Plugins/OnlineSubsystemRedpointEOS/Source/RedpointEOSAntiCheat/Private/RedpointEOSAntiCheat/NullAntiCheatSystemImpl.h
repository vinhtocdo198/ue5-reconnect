// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/Optional.h"
#include "RedpointEOSAntiCheat/AntiCheatSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3475017591, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;

class FNullAntiCheatSystemImpl : public IAntiCheatSystem, public TSharedFromThis<FNullAntiCheatSystemImpl>
{
public:
    FNullAntiCheatSystemImpl() = default;
    UE_NONCOPYABLE(FNullAntiCheatSystemImpl);
    virtual ~FNullAntiCheatSystemImpl() override = default;

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
REDPOINT_EOS_FILE_NS_EXPORT(3475017591, Redpoint::EOS::AntiCheat, FNullAntiCheatSystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()