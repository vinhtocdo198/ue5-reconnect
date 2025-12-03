// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_LOCK_PARTY FName("LockParty")

class FMatchmakingAlgorithmStepStateLockParty : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepLockParty : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateLockParty>,
                                           public TSharedFromThis<FMatchmakingAlgorithmStepLockParty>
{
private:
    void OnPartyLocked(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EUpdateConfigCompletionResult Result,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);
    void OnPartyStatsCallback(
        const FOnlineError &Error,
        const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
        TSharedRef<const FUniqueNetId> LocalUserId,
        TSharedRef<const FOnlinePartyId> PartyId,
        TUserIdSet UsersToLoadStatsFor,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);
    void OnSoloStatsCallback(
        const FOnlineError &ResultState,
        const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
        TUserIdSet UsersToLoadStatsFor,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepLockParty() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepLockParty);
    FMatchmakingAlgorithmStepLockParty() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_LOCK_PARTY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "LockParty_Status", "Starting matchmaking");
    }

    virtual FText GetDetail() const override
    {
        // @todo: Should we note that this *also* loads skill stats?
        return NSLOCTEXT("Matchmaking", "LockParty_Detail", "Locking party...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateLockParty> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()