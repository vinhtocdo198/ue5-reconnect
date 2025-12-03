// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineAsyncTaskManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)
#include "OnlineSubsystemSteam.h"
#endif

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem
{

#if defined(REDPOINT_EOS_STEAM_ENABLED)

class FOnlineSubsystemSteamProtectedAccessor : public FOnlineSubsystemSteam
{
public:
    static FOnlineAsyncTaskManager *GetAsyncTaskRunner(FOnlineSubsystemSteam *Steam);
};

#endif

}