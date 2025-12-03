// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_SEARCHING_PERFORM_SEARCH FName("SearchingPerformSearch")

class FMatchmakingAlgorithmStepStateSearchingPerformSearch : public IMatchmakingAlgorithmStepState
{
public:
    bool bInterruptedWithFollowRequest;
    FDateTime CompletionTime;
};

class FMatchmakingAlgorithmStepSearchingPerformSearch
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateSearchingPerformSearch>,
      public TSharedFromThis<FMatchmakingAlgorithmStepSearchingPerformSearch>
{
private:
    TSharedPtr<struct FMatchmakingCandidateLobby> LoadCandidateLobbyBasicInfo(
        FMatchmakingEngineTask &Task,
        const FOnlineLobbyId &InLobbyId,
        FString &UnsuitabilityReason,
        FString &StateHash);
    void OnSearchComplete(
        const FOnlineError &Error,
        const FUniqueNetId &UserId,
        const TArray<TSharedRef<const FOnlineLobbyId>> &Lobbies,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);
    void OnStatsCallback(
        const FOnlineError &ResultState,
        const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
        TArray<struct FMatchmakingSuitableMatchResult> AllCandidates,
        TArray<TSharedPtr<struct FMatchmakingCandidateLobby>> CandidateLobbies,
        TUserIdSet UsersToLoadStatsFor,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);
    void OnStatsLoaded(
        const FUniqueNetId &UserId,
        TArray<struct FMatchmakingSuitableMatchResult> AllCandidates,
        const TArray<TSharedPtr<struct FMatchmakingCandidateLobby>> &CandidateLobbies,
        const TWeakPtr<FMatchmakingEngineTask> &TaskWk);
    void RescheduleOnNoCandidatesFound(const TSharedRef<FMatchmakingEngineTask> &Task);
    void OnTimeout_WaitUntilFull(const TSharedRef<FMatchmakingEngineTask> &Task);
    void OnTimeout_CompletePartiallyFilled(const TSharedRef<FMatchmakingEngineTask> &Task);
    void OnTimeout_CompleteFillWithAI(const TSharedRef<FMatchmakingEngineTask> &Task);

public:
    virtual ~FMatchmakingAlgorithmStepSearchingPerformSearch() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepSearchingPerformSearch);
    FMatchmakingAlgorithmStepSearchingPerformSearch() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_SEARCHING_PERFORM_SEARCH;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "SearchingPerformSearch_Status", "Searching for other players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "SearchingPerformSearch_Detail", "Performing the search...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateSearchingPerformSearch> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;

    static TUserIdMap<FOpenskillRating> DeserializeOpenskillStats(
        const FString &SkillStatPrefix,
        const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
        const TUserIdSet &UsersToLoadStatsFor);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()