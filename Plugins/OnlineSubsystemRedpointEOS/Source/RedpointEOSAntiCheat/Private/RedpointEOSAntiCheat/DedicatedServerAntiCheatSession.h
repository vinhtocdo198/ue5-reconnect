// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_SERVER_CODE
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAntiCheat/AntiCheatPlayerTracker.h"
#include "RedpointEOSAntiCheat/AntiCheatSession.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace REDPOINT_EOS_FILE_NS_ID(2337843830, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;

class FDedicatedServerAntiCheatSession : public FAntiCheatSession
{
private:
    TSharedRef<FAntiCheatPlayerTracker> PlayerTracking;

public:
    int StackCount;

    FDedicatedServerAntiCheatSession();
    UE_NONCOPYABLE(FDedicatedServerAntiCheatSession);
    virtual ~FDedicatedServerAntiCheatSession() override = default;

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
REDPOINT_EOS_FILE_NS_EXPORT(2337843830, Redpoint::EOS::AntiCheat, FDedicatedServerAntiCheatSession)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()