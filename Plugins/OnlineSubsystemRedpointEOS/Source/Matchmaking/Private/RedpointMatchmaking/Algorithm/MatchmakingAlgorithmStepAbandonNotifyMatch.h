// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingAbandonmentType.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_ABANDON_NOTIFY_MATCH FName("AbandonNotifyMatch")

class FMatchmakingAlgorithmStepStateAbandonNotifyMatch : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepAbandonNotifyMatch
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateAbandonNotifyMatch, EMatchmakingAbandonmentType>,
      public TSharedFromThis<FMatchmakingAlgorithmStepAbandonNotifyMatch>
{
private:
    void OnMatchMembersNotified(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        EMatchmakingAbandonmentType AbandonmentType);

    void OnDisconnectedFromMatch(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        EMatchmakingAbandonmentType AbandonmentType);

    void OnDisconnectedFromFollowRequest(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        EMatchmakingAbandonmentType AbandonmentType);

public:
    virtual ~FMatchmakingAlgorithmStepAbandonNotifyMatch() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepAbandonNotifyMatch);
    FMatchmakingAlgorithmStepAbandonNotifyMatch() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_ABANDON_NOTIFY_MATCH;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonNotifyMatch_Status", "Please wait");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonNotifyMatch_Detail", "Please wait...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateAbandonNotifyMatch> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, EMatchmakingAbandonmentType AbandonmentType) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()