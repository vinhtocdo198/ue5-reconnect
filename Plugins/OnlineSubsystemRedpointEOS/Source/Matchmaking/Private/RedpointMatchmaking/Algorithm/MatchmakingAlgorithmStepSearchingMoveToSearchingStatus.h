// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_SEARCHING_MOVE_TO_SEARCHING_STATUS FName("SearchingMoveToSearchingStatus")

class FMatchmakingAlgorithmStepStateSearchingMoveToSearchingStatus : public IMatchmakingAlgorithmStepState
{
public:
    bool bInterruptedWithFollowRequest;
};

class FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateSearchingMoveToSearchingStatus>,
      public TSharedFromThis<FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus>
{
private:
    void OnLobbyUpdated(const FOnlineError &Error, const FUniqueNetId &UserId, TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus);
    FMatchmakingAlgorithmStepSearchingMoveToSearchingStatus() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_SEARCHING_MOVE_TO_SEARCHING_STATUS;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "SearchingMoveToSearchingStatus_Status", "Searching for other players");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "SearchingMoveToSearchingStatus_Detail", "Starting the search...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateSearchingMoveToSearchingStatus> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()