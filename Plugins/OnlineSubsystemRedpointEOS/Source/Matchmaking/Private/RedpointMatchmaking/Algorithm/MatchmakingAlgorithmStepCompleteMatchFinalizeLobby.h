// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_COMPLETE_MATCH_FINALIZE_LOBBY FName("CompleteMatchFinalizeLobby")

class FMatchmakingAlgorithmStepStateCompleteMatchFinalizeLobby : public IMatchmakingAlgorithmStepState
{
public:
    bool bHasFinishedLobbyUpdate;
    FDelegateHandle OnTimeoutHandle;
};

class FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateCompleteMatchFinalizeLobby>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby>
{
private:
    void CheckLobbyFinalized(const TSharedRef<FMatchmakingEngineTask> &Task);

    void OnTimeout(TWeakPtr<FMatchmakingEngineTask> TaskWk);

    void OnLobbyUpdated(const FOnlineError &Error, const FUniqueNetId &UserId, TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby);
    FMatchmakingAlgorithmStepCompleteMatchFinalizeLobby() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_COMPLETE_MATCH_FINALIZE_LOBBY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchFinalizeLobby_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteMatchFinalizeLobby_Detail", "Finalizing game lobby...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchFinalizeLobby> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()