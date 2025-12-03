// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

IMPLEMENT_MATCHMAKING_AUTOMATION_TEST(
    FMatchmaking_WithSingleSlotTest,
    "Redpoint.Matchmaking.WithSingleSlotTest",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter,
    1);

FMatchmakingEngineRequestHandle FMatchmaking_WithSingleSlotTest::QueueRequestForInstance(
    int InstanceIdx,
    TArray<FInstance> Instances,
    FMatchmakingRequestComplete OnDone)
{
    TSharedPtr<FMatchmakingEngineRequestHostConfiguration> HostConfiguration =
        MakeShared<FMatchmakingEngineRequestHostConfiguration>();
    HostConfiguration->QueueName = TEXT("Default");
    HostConfiguration->RequestId = 0;
    HostConfiguration->TeamCapacities.Add(1);
    HostConfiguration->OnNoCandidates = EMatchmakingBehaviourOnNoCandidates::WaitUntilFull;
    HostConfiguration->MinimumWaitSecondsBaseline = 0;
    HostConfiguration->MinimumWaitSecondsPerEmptySlot = 0;
    HostConfiguration->BalanceMode = EMatchmakingBalanceMode::MaximizeTeamFill;

    FMatchmakingEngineRequest Request = {};
    Request.Identity = Instances[InstanceIdx].OnlineSubsystem->GetIdentityInterface();
    Request.Lobby = Online::GetLobbyInterface(Instances[InstanceIdx].OnlineSubsystem.Get());
    Request.PartySystem = Instances[InstanceIdx].OnlineSubsystem->GetPartyInterface();
    Request.UserId = Instances[InstanceIdx].UserId;
    Request.PartyId = nullptr;
    Request.HostConfiguration = HostConfiguration;
    Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::ReturnResults;
    Request.CompletionBehaviour.OnComplete = FMatchmakingEngineRequestComplete::CreateLambda(
        [OnDone](const FString &TaskId, const FMatchmakingEngineRequest &, FMatchmakingEngineResponse Response) {
            UE_LOG(LogMatchmakingTests, Verbose, TEXT("Matchmaking unit test is complete"));
            OnDone.ExecuteIfBound();
        });
    Request.OnProgress = FMatchmakingEngineRequestProgress::CreateLambda(
        [](const FString &TaskId, const FMatchmakingEngineRequest &, FMatchmakingEngineProgressInfo ProgressInfo) {
            UE_LOG(
                LogMatchmakingTests,
                Verbose,
                TEXT("Matchmaking unit test got update from matchmaker: %s"),
                *ProgressInfo.StepName.ToString());
        });
    Request.OnCancelled = FMatchmakingEngineRequestCancelled::CreateLambda(
        [this, Instances, OnDone](const FString &TaskId, const FMatchmakingEngineRequest &) {
            this->TestFalse(TEXT("Did not expect to get OnCancelled!"), true);
            OnDone.ExecuteIfBound();
        });
    Request.OnError = FMatchmakingEngineRequestError::CreateLambda(
        [this, OnDone](const FString &TaskId, const FMatchmakingEngineRequest &, const FOnlineError &Error) {
            this->TestTrue(FString::Printf(TEXT("Did not expect to get error: %s"), *Error.ToLogString()), false);
            OnDone.ExecuteIfBound();
        });

    return this->Engine->Enqueue(Request);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()