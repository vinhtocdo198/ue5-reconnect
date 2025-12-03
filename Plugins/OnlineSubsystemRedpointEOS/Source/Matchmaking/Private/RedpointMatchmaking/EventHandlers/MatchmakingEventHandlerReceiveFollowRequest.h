// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "MatchmakingEventHandler.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST FName("WantToAcceptFollowRequest")

class FMatchmakingEventHandlerReceiveFollowRequest : public IMatchmakingEventHandler
{
private:
    void OnFollowRequestTimerElapsed(
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        TSharedRef<FMatchmakingEngineImpl> Engine,
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnMemberUpdate(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingEventHandlerReceiveFollowRequest() override = default;
    UE_NONCOPYABLE(FMatchmakingEventHandlerReceiveFollowRequest);
    FMatchmakingEventHandlerReceiveFollowRequest() = default;

    virtual void RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
    virtual void UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()