// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#include "UObject/SoftObjectPtr.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_RESERVE_DEDICATED_SERVER_FOR_MATCH FName("ReserveDedicatedServerForMatch")

class FMatchmakingAlgorithmStepStateReserveDedicatedServerForMatch : public IMatchmakingAlgorithmStepState
{
public:
    // This is only stored so we can clean it up if we get an abandonment signal.
    TSoftObjectPtr<class ADedicatedServerMatchmakingBeaconClient> BeaconActor;
};

class FMatchmakingAlgorithmStepReserveDedicatedServerForMatch
    : public IMatchmakingAlgorithmStep<
          FMatchmakingAlgorithmStepStateReserveDedicatedServerForMatch,
          FOnlineSessionSearchResult>,
      public TSharedFromThis<FMatchmakingAlgorithmStepReserveDedicatedServerForMatch>
{
private:
    void DestroyAndCleanupBeacon(FMatchmakingEngineTask &Task);

    void OnConnected(
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSoftObjectPtr<class ADedicatedServerMatchmakingBeaconClient> BeaconWk,
        FOnlineSessionSearchResult SearchResult);
    void OnReserveForMatchResult(
        bool bWasSuccessful,
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSoftObjectPtr<class ADedicatedServerMatchmakingBeaconClient> BeaconWk,
        FOnlineSessionSearchResult SearchResult);

    void OnHostConnectionFailure(
        TWeakPtr<FMatchmakingEngineTask> TaskWk,
        TSoftObjectPtr<class ADedicatedServerMatchmakingBeaconClient> BeaconWk,
        FOnlineSessionSearchResult SearchResult);

public:
    virtual ~FMatchmakingAlgorithmStepReserveDedicatedServerForMatch() override = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepReserveDedicatedServerForMatch);
    FMatchmakingAlgorithmStepReserveDedicatedServerForMatch() = default;

    virtual FName GetName() const override
    {
        return MMSTEP_RESERVE_DEDICATED_SERVER_FOR_MATCH;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "ReserveDedicatedServerForMatch_Status", "Finding a server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT("Matchmaking", "ReserveDedicatedServerForMatch_Detail", "Reserving the game server...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateReserveDedicatedServerForMatch> CreateNewState() override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task, FOnlineSessionSearchResult SearchResult) override;

    virtual bool CanHandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
    virtual bool HandleInterrupt(const TSharedRef<FMatchmakingEngineTask> &Task, FName InterruptType) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()