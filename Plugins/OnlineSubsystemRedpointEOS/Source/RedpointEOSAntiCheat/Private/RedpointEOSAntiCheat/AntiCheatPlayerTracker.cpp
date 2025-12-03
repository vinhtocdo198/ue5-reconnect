// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/AntiCheatPlayerTracker.h"

#include "RedpointEOSAntiCheat/Logging.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2509056333, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Id;

FAntiCheatPlayerTracker::FAntiCheatPlayerTracker()
    : NextHandle(reinterpret_cast<EOS_AntiCheatCommon_ClientHandle>(1000))
    , HandlesToPlayers()
    , PlayersToHandles()
    , PlayersToHandleCount()
{
}

EOS_AntiCheatCommon_ClientHandle FAntiCheatPlayerTracker::AddPlayer(
    UE::Online::FAccountId UserId,
    bool &bOutShouldRegister)
{
    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT("Player tracking: %s: Requested to add player"),
        *GetUserIdDebugString(UserId));

    if (this->PlayersToHandles.Contains(UserId))
    {
        this->PlayersToHandleCount[UserId]++;
        bOutShouldRegister = false;

        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT("Player tracking: %s: Incremented handle count as this player was already registered, handle count is "
                 "now at %d"),
            *GetUserIdDebugString(UserId),
            this->PlayersToHandleCount[UserId]);

        return this->PlayersToHandles[UserId];
    }
    else
    {
        EOS_AntiCheatCommon_ClientHandle ReturnHandle = this->NextHandle;
        this->NextHandle =
            reinterpret_cast<EOS_AntiCheatCommon_ClientHandle>(reinterpret_cast<intptr_t>(this->NextHandle) + 1);
        this->PlayersToHandles.Add(UserId, ReturnHandle);
        this->PlayersToHandleCount.Add(UserId, 1);
        this->HandlesToPlayers.Add(ReturnHandle, UserId);
        bOutShouldRegister = true;

        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT("Player tracking: %s: Added new player, handle count is now at %d"),
            *GetUserIdDebugString(UserId),
            this->PlayersToHandleCount[UserId]);

        return ReturnHandle;
    }
}

void FAntiCheatPlayerTracker::RemovePlayer(UE::Online::FAccountId UserId)
{
    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT("Player tracking: %s: Requested to remove player"),
        *GetUserIdDebugString(UserId));

    if (!this->PlayersToHandles.Contains(UserId))
    {
        // Player isn't currently registered.
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT("Player tracking: %s: Player was not registered, ignoring remove request"),
            *GetUserIdDebugString(UserId));
        return;
    }

    this->PlayersToHandleCount[UserId]--;

    if (this->PlayersToHandleCount[UserId] == 0)
    {
        // We're at zero, also deregister.
        EOS_AntiCheatCommon_ClientHandle Handle = this->PlayersToHandles[UserId];
        this->PlayersToHandles.Remove(UserId);
        this->PlayersToHandleCount.Remove(UserId);
        this->HandlesToPlayers.Remove(Handle);

        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT("Player tracking: %s: Removed player as handle count reached 0"),
            *GetUserIdDebugString(UserId));
    }
    else
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT("Player tracking: %s: Decremented handle count, handle count is now at %d"),
            *GetUserIdDebugString(UserId),
            this->PlayersToHandleCount[UserId]);
    }
}

bool FAntiCheatPlayerTracker::ShouldDeregisterPlayerBeforeRemove(UE::Online::FAccountId UserId) const
{
    if (!this->PlayersToHandles.Contains(UserId))
    {
        return false;
    }

    return this->PlayersToHandleCount[UserId] == 1;
}

TOptional<UE::Online::FAccountId> FAntiCheatPlayerTracker::GetPlayer(EOS_AntiCheatCommon_ClientHandle Handle) const
{
    auto *Value = this->HandlesToPlayers.Find(Handle);
    if (Value != nullptr)
    {
        return *Value;
    }
    return TOptional<UE::Online::FAccountId>();
}

EOS_AntiCheatCommon_ClientHandle FAntiCheatPlayerTracker::GetHandle(UE::Online::FAccountId UserId) const
{
    return this->PlayersToHandles[UserId];
}

bool FAntiCheatPlayerTracker::HasPlayer(UE::Online::FAccountId UserId) const
{
    return this->PlayersToHandles.Contains(UserId);
}

}

REDPOINT_EOS_CODE_GUARD_END()