// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "MatchmakingEventHandler.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMINTERRUPT_PARTY_CHANGE FName("PartyChange")

class FMatchmakingEventHandlerPartyChange : public IMatchmakingEventHandler
{
private:
    void OnPartyExited(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);
    void OnPartyMemberJoined(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);
    void OnPartyMemberExited(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const EMemberExitedReason Reason,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);
    void OnPartyMemberPromoted(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &NewLeaderId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingEventHandlerPartyChange() override = default;
    UE_NONCOPYABLE(FMatchmakingEventHandlerPartyChange);
    FMatchmakingEventHandlerPartyChange() = default;

    virtual void RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
    virtual void UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()