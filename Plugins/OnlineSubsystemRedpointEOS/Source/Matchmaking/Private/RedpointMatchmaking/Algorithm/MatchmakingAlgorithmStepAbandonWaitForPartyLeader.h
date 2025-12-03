// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingAbandonmentType.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_ABANDON_WAIT_FOR_PARTY_LEADER FName("AbandonWaitForPartyLeader")

class FMatchmakingAlgorithmStepStateAbandonWaitForPartyLeader : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle OnPartyDataReceivedDelegateHandle;
};

class FMatchmakingAlgorithmStepAbandonWaitForPartyLeader
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateAbandonWaitForPartyLeader>,
      public TSharedFromThis<FMatchmakingAlgorithmStepAbandonWaitForPartyLeader>
{
private:
    void OnPartyDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepAbandonWaitForPartyLeader() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepAbandonWaitForPartyLeader);
    FMatchmakingAlgorithmStepAbandonWaitForPartyLeader() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_ABANDON_WAIT_FOR_PARTY_LEADER;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonWaitForPartyLeader_Status", "Please wait");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonWaitForPartyLeader_Detail", "Please wait...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateAbandonWaitForPartyLeader> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()