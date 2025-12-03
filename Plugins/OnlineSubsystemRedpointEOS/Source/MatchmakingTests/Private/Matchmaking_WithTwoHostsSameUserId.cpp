// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

IMPLEMENT_ADVANCED_MATCHMAKING_AUTOMATION_TEST(
    FMatchmaking_WithTwoHostsSameUserId,
    "Redpoint.Matchmaking.WithTwoHostsSameUserId",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter,
    2);

TSharedPtr<const FUniqueNetId> FMatchmaking_WithTwoHostsSameUserId::GetUserId(int InstanceIdx)
{
    return MakeShared<const FUniqueNetIdMMTest>(0);
}

FMatchmakingEngineRequestHandle FMatchmaking_WithTwoHostsSameUserId::QueueRequestForInstance(
    int InstanceIdx,
    TArray<FInstance> Instances,
    FMatchmakingRequestComplete OnDone)
{
    TSharedPtr<FMatchmakingEngineRequestHandle> HandleHolder = MakeShared<FMatchmakingEngineRequestHandle>();
    TSharedPtr<int> ProgressCounter = MakeShared<int>(0);

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
    HostConfiguration->OnCandidateRejected =
        FMatchmakingEngineRequestCandidateRejected::CreateLambda([Engine = this->Engine, HandleHolder, ProgressCounter](
                                                                     const FString &TaskId,
                                                                     const struct FMatchmakingEngineRequest &Request,
                                                                     const FString &CandidateLobbyId,
                                                                     const FString &CandidateRejectionReason) {
            if (CandidateRejectionReason == TEXT("shares an existing member with us"))
            {
                Engine->Cancel(*HandleHolder);
            }
        });

    FMatchmakingEngineRequest Request = {};
    Request.Identity = Instances[InstanceIdx].OnlineSubsystem->GetIdentityInterface();
    Request.Lobby = Online::GetLobbyInterface(Instances[InstanceIdx].OnlineSubsystem.Get());
    Request.PartySystem = Instances[InstanceIdx].OnlineSubsystem->GetPartyInterface();
    Request.UserId = Instances[InstanceIdx].UserId;
    Request.PartyId = nullptr;
    Request.HostConfiguration = HostConfiguration;
    Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::ReturnResults;
    Request.CompletionBehaviour.OnComplete = FMatchmakingEngineRequestComplete::CreateLambda(
        [this, OnDone](const FString &TaskId, const FMatchmakingEngineRequest &, FMatchmakingEngineResponse Response) {
            this->TestTrue(TEXT("Did not expect this test to complete matchmaking"), false);
            OnDone.ExecuteIfBound();
        });
    Request.OnProgress = FMatchmakingEngineRequestProgress::CreateLambda(
        [this, InstanceIdx, Engine = this->Engine, ProgressCounter, HandleHolder](
            const FString &TaskId,
            const FMatchmakingEngineRequest &,
            FMatchmakingEngineProgressInfo ProgressInfo) {
            if (InstanceIdx == 0)
            {
                if (ProgressInfo.StepName.ToString() == TEXT("Searching"))
                {
                    (*ProgressCounter)++;
                }

                if (*ProgressCounter > 10)
                {
                    this->TestFalse(TEXT("Expected to eliminate candidate due to sharing a member"), true);
                    Engine->Cancel(*HandleHolder);
                }
            }
            else
            {
                if (ProgressInfo.StepName.ToString() == TEXT("Searching"))
                {
                    (*ProgressCounter)++;
                }

                // We expect to have to cancel on the client because the host cancelled.
                if (*ProgressCounter > 20)
                {
                    Engine->Cancel(*HandleHolder);
                }
            }
        });
    Request.OnCancelled = FMatchmakingEngineRequestCancelled::CreateLambda(
        [OnDone](const FString &TaskId, const FMatchmakingEngineRequest &) {
            // We expect to get cancelled.
            OnDone.ExecuteIfBound();
        });
    Request.OnError = FMatchmakingEngineRequestError::CreateLambda(
        [this, OnDone](const FString &TaskId, const FMatchmakingEngineRequest &, const FOnlineError &Error) {
            this->TestTrue(FString::Printf(TEXT("Did not expect to get error: %s"), *Error.ToLogString()), false);
            OnDone.ExecuteIfBound();
        });

    *HandleHolder = Engine->Enqueue(Request);
    return *HandleHolder;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()