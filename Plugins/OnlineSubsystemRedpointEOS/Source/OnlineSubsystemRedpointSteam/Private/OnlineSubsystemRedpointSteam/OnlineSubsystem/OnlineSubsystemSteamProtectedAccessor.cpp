// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemSteamProtectedAccessor.h"

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem
{

#if defined(REDPOINT_EOS_STEAM_ENABLED)

FOnlineAsyncTaskManager *FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner(FOnlineSubsystemSteam *Steam)
{
    return (
        FOnlineAsyncTaskManager *)(((FOnlineSubsystemSteamProtectedAccessor *)Steam)->OnlineAsyncTaskThreadRunnable);
}

#endif

}