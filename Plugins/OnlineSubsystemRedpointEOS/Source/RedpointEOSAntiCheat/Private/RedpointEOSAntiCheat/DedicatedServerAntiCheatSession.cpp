// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/DedicatedServerAntiCheatSession.h"

#if WITH_SERVER_CODE
#include "RedpointEOSAntiCheat/AntiCheatPlayerTracker.h"
#include "RedpointEOSAntiCheat/Logging.h"
#include "RedpointEOSCore/Id/Id.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace REDPOINT_EOS_FILE_NS_ID(2337843830, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Id;

FDedicatedServerAntiCheatSession::FDedicatedServerAntiCheatSession()
    : FAntiCheatSession()
    , PlayerTracking(MakeShared<FAntiCheatPlayerTracker>())
    , StackCount(0)
{
}

EOS_AntiCheatCommon_ClientHandle FDedicatedServerAntiCheatSession::AddPlayer(
    UE::Online::FAccountId UserId,
    bool &bOutShouldRegister)
{
    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT("Dedicated server Anti-Cheat: %p: AddPlayer(UserId: %s)"),
        this,
        *GetUserIdDebugString(UserId));

    return this->PlayerTracking->AddPlayer(UserId, bOutShouldRegister);
}

void FDedicatedServerAntiCheatSession::RemovePlayer(UE::Online::FAccountId UserId)
{
    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT("Dedicated server Anti-Cheat: %p: RemovePlayer(UserId: %s)"),
        this,
        *GetUserIdDebugString(UserId));

    this->PlayerTracking->RemovePlayer(UserId);
}

bool FDedicatedServerAntiCheatSession::ShouldDeregisterPlayerBeforeRemove(UE::Online::FAccountId UserId) const
{
    return this->PlayerTracking->ShouldDeregisterPlayerBeforeRemove(UserId);
}

TOptional<UE::Online::FAccountId> FDedicatedServerAntiCheatSession::GetPlayer(EOS_AntiCheatCommon_ClientHandle Handle)
{
    return this->PlayerTracking->GetPlayer(Handle);
}

EOS_AntiCheatCommon_ClientHandle FDedicatedServerAntiCheatSession::GetHandle(UE::Online::FAccountId UserId)
{
    return this->PlayerTracking->GetHandle(UserId);
}

bool FDedicatedServerAntiCheatSession::HasPlayer(UE::Online::FAccountId UserId)
{
    return this->PlayerTracking->HasPlayer(UserId);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()