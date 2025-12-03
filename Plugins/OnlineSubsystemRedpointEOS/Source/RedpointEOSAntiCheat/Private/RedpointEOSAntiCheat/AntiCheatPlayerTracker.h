// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/AccountIdMap.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2509056333, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::Core::Id;

class FAntiCheatPlayerTracker
{
private:
    EOS_AntiCheatCommon_ClientHandle NextHandle;
    TMap<EOS_AntiCheatCommon_ClientHandle, UE::Online::FAccountId> HandlesToPlayers;
    TAccountIdMap<EOS_AntiCheatCommon_ClientHandle> PlayersToHandles;
    TAccountIdMap<int> PlayersToHandleCount;

public:
    FAntiCheatPlayerTracker();

    EOS_AntiCheatCommon_ClientHandle AddPlayer(UE::Online::FAccountId UserId, bool &bOutShouldRegister);
    void RemovePlayer(UE::Online::FAccountId UserId);
    bool ShouldDeregisterPlayerBeforeRemove(UE::Online::FAccountId UserId) const;

    TOptional<UE::Online::FAccountId> GetPlayer(EOS_AntiCheatCommon_ClientHandle Handle) const;
    EOS_AntiCheatCommon_ClientHandle GetHandle(UE::Online::FAccountId UserId) const;
    bool HasPlayer(UE::Online::FAccountId UserId) const;
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(2509056333, Redpoint::EOS::AntiCheat, FAntiCheatPlayerTracker)
}

REDPOINT_EOS_CODE_GUARD_END()