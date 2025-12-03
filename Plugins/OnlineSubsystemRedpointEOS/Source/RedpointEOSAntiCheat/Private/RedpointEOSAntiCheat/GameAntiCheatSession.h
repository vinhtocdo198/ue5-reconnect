// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "Misc/Optional.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAntiCheat/AntiCheatPlayerTracker.h"
#include "RedpointEOSAntiCheat/AntiCheatSession.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace REDPOINT_EOS_FILE_NS_ID(3615136329, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;

class FGameAntiCheatSession : public FAntiCheatSession
{
private:
    TSharedRef<FAntiCheatPlayerTracker> PlayerTracking;

public:
    bool bIsServer;
    UE::Online::FAccountId HostUserId;
    TOptional<UE::Online::FAccountId> ListenServerUserId;
    bool bIsDedicatedServerSession;
    int StackCount;
    FString ServerConnectionUrlOnClient;

    FGameAntiCheatSession(UE::Online::FAccountId InHostUserId);
    UE_NONCOPYABLE(FGameAntiCheatSession);
    virtual ~FGameAntiCheatSession() override = default;

    EOS_AntiCheatCommon_ClientHandle AddPlayer(UE::Online::FAccountId UserId, bool &bOutShouldRegister);
    void RemovePlayer(UE::Online::FAccountId UserId);
    bool ShouldDeregisterPlayerBeforeRemove(UE::Online::FAccountId UserId) const;
    TOptional<UE::Online::FAccountId> GetPlayer(EOS_AntiCheatCommon_ClientHandle Handle);
    EOS_AntiCheatCommon_ClientHandle GetHandle(UE::Online::FAccountId UserId);
    bool HasPlayer(UE::Online::FAccountId UserId);
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(3615136329, Redpoint::EOS::AntiCheat, FGameAntiCheatSession)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()