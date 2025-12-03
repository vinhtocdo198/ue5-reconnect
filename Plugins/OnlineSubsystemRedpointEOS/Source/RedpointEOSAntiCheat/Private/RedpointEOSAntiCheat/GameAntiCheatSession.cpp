// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/GameAntiCheatSession.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAntiCheat/Logging.h"
#include "RedpointEOSCore/Id/Id.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace REDPOINT_EOS_FILE_NS_ID(3615136329, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Id;

FGameAntiCheatSession::FGameAntiCheatSession(UE::Online::FAccountId InHostUserId)
    : FAntiCheatSession()
    , PlayerTracking(MakeShared<FAntiCheatPlayerTracker>())
    , bIsServer(false)
    , HostUserId(InHostUserId)
    , ListenServerUserId()
    , bIsDedicatedServerSession(false)
    , StackCount(0)
    , ServerConnectionUrlOnClient(TEXT(""))
{
}

EOS_AntiCheatCommon_ClientHandle FGameAntiCheatSession::AddPlayer(
    UE::Online::FAccountId UserId,
    bool &bOutShouldRegister)
{
    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT("Game Anti-Cheat: %p: AddPlayer(UserId: %s)"),
        this,
        *GetUserIdDebugString(UserId));

    return this->PlayerTracking->AddPlayer(UserId, bOutShouldRegister);
}

void FGameAntiCheatSession::RemovePlayer(UE::Online::FAccountId UserId)
{
    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT("Game Anti-Cheat: %p: RemovePlayer(UserId: %s)"),
        this,
        *GetUserIdDebugString(UserId));

    this->PlayerTracking->RemovePlayer(UserId);
}

bool FGameAntiCheatSession::ShouldDeregisterPlayerBeforeRemove(UE::Online::FAccountId UserId) const
{
    return this->PlayerTracking->ShouldDeregisterPlayerBeforeRemove(UserId);
}

TOptional<UE::Online::FAccountId> FGameAntiCheatSession::GetPlayer(EOS_AntiCheatCommon_ClientHandle Handle)
{
    return this->PlayerTracking->GetPlayer(Handle);
}

EOS_AntiCheatCommon_ClientHandle FGameAntiCheatSession::GetHandle(UE::Online::FAccountId UserId)
{
    return this->PlayerTracking->GetHandle(UserId);
}

bool FGameAntiCheatSession::HasPlayer(UE::Online::FAccountId UserId)
{
    return this->PlayerTracking->HasPlayer(UserId);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()