// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

IMPLEMENT_MATCHMAKING_AUTOMATION_TEST(
    FMatchmaking_WithTwoHostsIn1v1,
    "Redpoint.Matchmaking.WithTwoHostsIn1v1",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter,
    2);

FMatchmakingEngineRequestHandle FMatchmaking_WithTwoHostsIn1v1::QueueRequestForInstance(
    int InstanceIdx,
    TArray<FInstance> Instances,
    FMatchmakingRequestComplete OnDone)
{
    TSharedPtr<FMatchmakingEngineRequestHostConfiguration> HostConfiguration =
        MakeShared<FMatchmakingEngineRequestHostConfiguration>();
    HostConfiguration->QueueName = TEXT("Default");
    HostConfiguration->RequestId = 1001 + InstanceIdx;
    HostConfiguration->TeamCapacities.Add(1);
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
    Request.CompletionBehaviour.OnComplete =
        FMatchmakingEngineRequestComplete::CreateLambda([this, InstanceIdx, Instances, OnDone](
                                                            const FString &TaskId,
                                                            const FMatchmakingEngineRequest &,
                                                            FMatchmakingEngineResponse Response) {
            UE_LOG(LogMatchmakingTests, Verbose, TEXT("Instance(%d) matchmaking unit test is complete"), InstanceIdx);
            ASSERT_MM_SLOT_USER(0, 0, 0);
            ASSERT_MM_SLOT_USER(1, 0, 1);
            OnDone.ExecuteIfBound();
        });
    Request.OnProgress = FMatchmakingEngineRequestProgress::CreateLambda([
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                                                             InstanceIdx
#endif
    ](const FString &TaskId, const FMatchmakingEngineRequest &, FMatchmakingEngineProgressInfo ProgressInfo) {
        UE_LOG(
            LogMatchmakingTests,
            Verbose,
            TEXT("Instance(%d) matchmaking unit test got update from matchmaker: %s"),
            InstanceIdx,
            *ProgressInfo.StepName.ToString());
    });
    Request.OnCancelled = FMatchmakingEngineRequestCancelled::CreateLambda(
        [this, OnDone](const FString &TaskId, const FMatchmakingEngineRequest &) {
            this->TestFalse(TEXT("Did not expect to get OnCancelled!"), true);
            OnDone.ExecuteIfBound();
        });
    Request.OnError = FMatchmakingEngineRequestError::CreateLambda(
        [this, OnDone](const FString &TaskId, const FMatchmakingEngineRequest &, const FOnlineError &Error) {
            this->TestTrue(FString::Printf(TEXT("Did not expect to get error: %s"), *Error.ToLogString()), false);
            OnDone.ExecuteIfBound();
        });

    return Engine->Enqueue(Request);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()