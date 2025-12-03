// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/NullAntiCheatSystemImpl.h"

#include "RedpointEOSAntiCheat/Logging.h"
#include "RedpointEOSAntiCheat/NullAntiCheatSession.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3475017591, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;

bool FNullAntiCheatSystemImpl::ShouldGameExitOnStartup() const
{
    return false;
}

EAntiCheatImplementationType FNullAntiCheatSystemImpl::GetImplementationType() const
{
    return EAntiCheatImplementationType::Null;
}

bool FNullAntiCheatSystemImpl::ShouldRemoteSkipPeerRegistration() const
{
    return false;
}

TSharedPtr<FAntiCheatSession> FNullAntiCheatSystemImpl::CreateSession(
    bool bIsServer,
    UE::Online::FAccountId HostUserId,
    bool bIsDedicatedServerSession,
    TOptional<UE::Online::FAccountId> ListenServerUserId,
    FString ServerConnectionUrlOnClient)
{
#if !(PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)
    UE_LOG(
        LogRedpointEOSAntiCheat,
        Warning,
        TEXT("This platform does not support hosting Anti-Cheat sessions. This game session will not be protected by "
             "Anti-Cheat, even for clients that connect on desktop platforms."))
#endif
    return MakeShared<FNullAntiCheatSession>();
}

bool FNullAntiCheatSystemImpl::DestroySession(FAntiCheatSession &Session)
{
    return true;
}

bool FNullAntiCheatSystemImpl::RegisterPlayer(
    FAntiCheatSession &Session,
    UE::Online::FAccountId UserId,
    EOS_EAntiCheatCommonClientType ClientType,
    EOS_EAntiCheatCommonClientPlatform ClientPlatform)
{
    this->OnPlayerAuthStatusChanged().Broadcast(
        UserId,
        EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_RemoteAuthComplete);
    return true;
}

bool FNullAntiCheatSystemImpl::UnregisterPlayer(FAntiCheatSession &Session, UE::Online::FAccountId UserId)
{
    return true;
}

bool FNullAntiCheatSystemImpl::ReceiveNetworkMessage(
    FAntiCheatSession &Session,
    UE::Online::FAccountId SourceUserId,
    UE::Online::FAccountId TargetUserId,
    const uint8 *Data,
    uint32_t Size)
{
    checkf(false, TEXT("Did not expect to receive network messages in null Anti-Cheat implementation."));
    return false;
}

}

REDPOINT_EOS_CODE_GUARD_END()