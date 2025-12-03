// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "MatchmakingEventHandler.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMINTERRUPT_KICKED FName("Kicked")

class FMatchmakingEventHandlerKicked : public IMatchmakingEventHandler
{
private:
    void OnLobbyMemberDisconnect(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        bool bWasKicked,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingEventHandlerKicked() override = default;
    UE_NONCOPYABLE(FMatchmakingEventHandlerKicked);
    FMatchmakingEventHandlerKicked() = default;

    virtual void RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
    virtual void UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()