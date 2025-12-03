// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "MatchmakingEventHandler.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMINTERRUPT_CLIENT_GOT_HOST_HEARTBEAT FName("HostHeartbeat")

class FMatchmakingEventHandlerHostHeartbeat : public IMatchmakingEventHandler
{
private:
    void OnLobbyUpdate(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingEventHandlerHostHeartbeat() override = default;
    UE_NONCOPYABLE(FMatchmakingEventHandlerHostHeartbeat);
    FMatchmakingEventHandlerHostHeartbeat() = default;

    virtual void RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
    virtual void UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()