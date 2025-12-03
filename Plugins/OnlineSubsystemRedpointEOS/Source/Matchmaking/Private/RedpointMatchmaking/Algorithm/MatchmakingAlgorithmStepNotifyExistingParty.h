// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_NOTIFY_EXISTING_PARTY FName("NotifyExistingParty")

class FMatchmakingAlgorithmStepStateNotifyExistingParty : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle OnPartyDataReceivedHandle;
};

class FMatchmakingAlgorithmStepNotifyExistingParty
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateNotifyExistingParty>,
      public TSharedFromThis<FMatchmakingAlgorithmStepNotifyExistingParty>
{
private:
    void OnPartyDataUpdated(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData,
        TWeakPtr<FMatchmakingEngineTask> TaskWk);

public:
    virtual ~FMatchmakingAlgorithmStepNotifyExistingParty() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepNotifyExistingParty);
    FMatchmakingAlgorithmStepNotifyExistingParty() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_NOTIFY_EXISTING_PARTY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "NotifyExistingParty_Status", "Starting matchmaking");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "NotifyExistingParty_Detail", "Notifying other party members...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateNotifyExistingParty> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()