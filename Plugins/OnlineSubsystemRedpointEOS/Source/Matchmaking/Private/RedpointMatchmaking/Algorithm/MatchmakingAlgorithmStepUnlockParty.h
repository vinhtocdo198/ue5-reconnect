// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingAbandonmentType.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_ABANDON_UNLOCK_PARTY FName("AbandonUnlockParty")
#define MMSTEP_COMPLETE_UNLOCK_PARTY FName("CompleteUnlockParty")

class FMatchmakingAlgorithmStepStateUnlockParty : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepUnlockParty
{
protected:
    DECLARE_DELEGATE(FOnInternalPartyUnlocked);

    void UnlockPartyIfNeeded(TSharedRef<FMatchmakingEngineTask> Task, FOnInternalPartyUnlocked OnComplete);
    virtual TSharedRef<FMatchmakingAlgorithmStepUnlockParty> AsSharedBase() = 0;

private:
    void OnPartyUnlocked(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EUpdateConfigCompletionResult Result,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        FOnInternalPartyUnlocked OnComplete);

public:
    FMatchmakingAlgorithmStepUnlockParty() = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepUnlockParty);
    virtual ~FMatchmakingAlgorithmStepUnlockParty() = default;
};

class FMatchmakingAlgorithmStepAbandonUnlockParty
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateUnlockParty, EMatchmakingAbandonmentType>,
      public TSharedFromThis<FMatchmakingAlgorithmStepAbandonUnlockParty>,
      public FMatchmakingAlgorithmStepUnlockParty
{
private:
    void OnComplete(TWeakPtr<FMatchmakingEngineTask> TaskWk, EMatchmakingAbandonmentType AbandonmentType);

protected:
    virtual TSharedRef<FMatchmakingAlgorithmStepUnlockParty> AsSharedBase() override;

public:
    virtual ~FMatchmakingAlgorithmStepAbandonUnlockParty() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepAbandonUnlockParty);
    FMatchmakingAlgorithmStepAbandonUnlockParty() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_ABANDON_UNLOCK_PARTY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonUnlockParty_Status", "Please wait");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonUnlockParty_Detail", "Please wait...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateUnlockParty> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, EMatchmakingAbandonmentType AbandonmentType) override;
};

class FMatchmakingAlgorithmStepCompleteUnlockParty
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateUnlockParty>,
      public TSharedFromThis<FMatchmakingAlgorithmStepCompleteUnlockParty>,
      public FMatchmakingAlgorithmStepUnlockParty
{
private:
    void OnComplete(TWeakPtr<FMatchmakingEngineTask> TaskWk);

protected:
    virtual TSharedRef<FMatchmakingAlgorithmStepUnlockParty> AsSharedBase() override;

public:
    virtual ~FMatchmakingAlgorithmStepCompleteUnlockParty() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepCompleteUnlockParty);
    FMatchmakingAlgorithmStepCompleteUnlockParty() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_COMPLETE_UNLOCK_PARTY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteUnlockParty_Status", "Finishing up");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "CompleteUnlockParty_Detail", "Unlocking party after matchmaking...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateUnlockParty> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()