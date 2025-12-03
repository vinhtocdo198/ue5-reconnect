// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingAbandonmentType.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_ABANDON_NOTIFY_PARTY FName("AbandonNotifyParty")

class FMatchmakingAlgorithmStepStateAbandonNotifyParty : public IMatchmakingAlgorithmStepState
{
public:
    FDelegateHandle OnPartyMemberDataReceivedDelegateHandle;
    FDelegateHandle OnPartyDataReceivedDelegateHandle;
};

class FMatchmakingAlgorithmStepAbandonNotifyParty
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateAbandonNotifyParty, EMatchmakingAbandonmentType>,
      public TSharedFromThis<FMatchmakingAlgorithmStepAbandonNotifyParty>
{
private:
    void OnPartyMemberDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const FName &Namespace,
        const FOnlinePartyData &PartyMemberData,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        EMatchmakingAbandonmentType AbandonmentType,
        FString ExpectedValue);

    void OnPartyDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        EMatchmakingAbandonmentType AbandonmentType,
        FString ExpectedValue);

public:
    virtual ~FMatchmakingAlgorithmStepAbandonNotifyParty() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepAbandonNotifyParty);
    FMatchmakingAlgorithmStepAbandonNotifyParty() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_ABANDON_NOTIFY_PARTY;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonNotifyParty_Status", "Please wait");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "AbandonNotifyParty_Detail", "Please wait...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateAbandonNotifyParty> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, EMatchmakingAbandonmentType AbandonmentType) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()