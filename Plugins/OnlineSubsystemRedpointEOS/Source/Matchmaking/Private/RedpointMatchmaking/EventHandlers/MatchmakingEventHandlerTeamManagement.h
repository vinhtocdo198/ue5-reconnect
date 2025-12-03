// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "MatchmakingEventHandler.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMINTERRUPT_ALL_PLAYERS_HAVE_CONNECTED FName("AllPlayersHaveConnected")

class FMatchmakingEventHandlerTeamManagement : public IMatchmakingEventHandler
{
private:
    void OnMemberConnect_LobbyUpdated(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedRef<const FUniqueNetId> MemberId,
        FMatchmakingTeamId Team);

    void OnMemberConnect(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnMemberDisconnect_LobbyUpdated(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedRef<const FUniqueNetId> MemberId,
        FMatchmakingTeamId Team);

    void OnMemberDisconnect(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        bool bWasKicked,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnMemberConnect_MemberKicked(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TSharedRef<FMatchmakingEngineImpl> Engine,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSharedRef<const FUniqueNetId> MemberId);

public:
    virtual ~FMatchmakingEventHandlerTeamManagement() override = default;
    UE_NONCOPYABLE(FMatchmakingEventHandlerTeamManagement);
    FMatchmakingEventHandlerTeamManagement() = default;

    virtual void RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
    virtual void UnregisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()