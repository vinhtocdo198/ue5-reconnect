// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "MatchmakingEventHandler.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT FName("HasSpecialHandlingOfAbandonment")
#define MMINTERRUPT_ABANDON_CANCEL FName("AbandonCancel")
#define MMINTERRUPT_ABANDON_RESTART FName("AbandonRestart")

class FMatchmakingEventHandlerAbandonment : public IMatchmakingEventHandler
{
private:
    void OnLobbyMemberUpdate(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnPartyUpdate(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnPartyMemberUpdate(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const FName &Namespace,
        const FOnlinePartyData &PartyMemberData,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingEventHandlerAbandonment() override = default;
    UE_NONCOPYABLE(FMatchmakingEventHandlerAbandonment);
    FMatchmakingEventHandlerAbandonment() = default;

    virtual void RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
    virtual void UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()