// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "Delegates/DelegateCombinations.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "MatchmakerSubsystem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "MatchmakerSubmitMatchResults.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchmakerSubmitMatchResultsComplete, bool, bWasSuccessful);

UCLASS()
class MATCHMAKINGMATCHMAKER_API UMatchmakerSubmitMatchResults : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Matchmaking")
    FMatchmakerSubmitMatchResultsComplete OnComplete;

    UFUNCTION(BlueprintCallable, Category = "Matchmaking", meta = (BlueprintInternalUseOnly = "true"))
    static UMatchmakerSubmitMatchResults *SubmitMatchResults(
        UMatchmakerSubsystem *Subsystem,
        FUniqueNetIdRepl LocalUserId,
        TArray<float> TeamScores);

    virtual void Activate() override;

private:
#if !REDPOINT_EOS_IS_FREE_EDITION
    void OnCallback(const FOnlineError &Error);
#endif

    UPROPERTY()
    TObjectPtr<UMatchmakerSubsystem> Subsystem;

    UPROPERTY()
    FUniqueNetIdRepl LocalUserId;

    UPROPERTY()
    TArray<float> TeamScores;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()