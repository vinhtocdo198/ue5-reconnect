// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamDelegates.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(500868213, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

class FOnlineAsyncTaskSteamGetLargeFriendAvatar : public FOnlineAsyncTaskBasic<class FOnlineSubsystemSteam>
{
private:
    bool bInit;
    uint64 SteamID;
    FSteamAvatarDataFetched Delegate;
    uint32 ResultWidth;
    uint32 ResultHeight;
    TArray<uint8> ResultRGBA;
    FOnlineError ResultError;
    bool bHasChecked;
    bool bWasImmediatelyAvailable;

public:
    FOnlineAsyncTaskSteamGetLargeFriendAvatar(uint64 InSteamID, FSteamAvatarDataFetched InDelegate);
    UE_NONCOPYABLE(FOnlineAsyncTaskSteamGetLargeFriendAvatar);
    virtual ~FOnlineAsyncTaskSteamGetLargeFriendAvatar() = default;

    virtual FString ToString() const override
    {
        return TEXT("FOnlineAsyncTaskSteamGetLargeFriendAvatar");
    }

    virtual void Tick() override;
    virtual void Finalize() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks
{
REDPOINT_EOS_FILE_NS_EXPORT(
    500868213,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks,
    FOnlineAsyncTaskSteamGetLargeFriendAvatar)
}

#endif