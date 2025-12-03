// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

IMPLEMENT_MATCHMAKING_PARTY_AUTOMATION_TEST(
    FMatchmaking_WithCancellationAtFollowHost,
    "Redpoint.Matchmaking.WithCancellationAtFollowHost",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter,
    "3:2:2:1:1:1");

FMatchmakingEngineRequestHandle FMatchmaking_WithCancellationAtFollowHost::QueueRequestForInstance(
    int InstanceIdx,
    TSharedPtr<const FOnlinePartyId> PartyId,
    bool bIsPartyLeader,
    TArray<FInstance> Instances,
    FMatchmakingRequestComplete OnDone)
{
    FMatchmakingEngineRequest Request = {};
    Request.Identity = Instances[InstanceIdx].OnlineSubsystem->GetIdentityInterface();
    Request.Lobby = Online::GetLobbyInterface(Instances[InstanceIdx].OnlineSubsystem.Get());
    Request.PartySystem = Instances[InstanceIdx].OnlineSubsystem->GetPartyInterface();
    Request.UserId = Instances[InstanceIdx].UserId;
    Request.PartyId = PartyId;

    if (bIsPartyLeader)
    {
        TSharedPtr<FMatchmakingEngineRequestHostConfiguration> HostConfiguration =
            MakeShared<FMatchmakingEngineRequestHostConfiguration>();
        HostConfiguration->QueueName = TEXT("Default");
        HostConfiguration->RequestId = 1001 + InstanceIdx;
        HostConfiguration->TeamCapacities.Add(4);
        HostConfiguration->TeamCapacities.Add(4);
        HostConfiguration->OnNoCandidates = EMatchmakingBehaviourOnNoCandidates::WaitUntilFull;
        HostConfiguration->MinimumWaitSecondsBaseline = 0;
        HostConfiguration->MinimumWaitSecondsPerEmptySlot = 0;
        HostConfiguration->BalanceMode = EMatchmakingBalanceMode::MaximizeTeamFill;
        Request.HostConfiguration = HostConfiguration;
    }

    Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::ReturnResults;
    Request.CompletionBehaviour.OnComplete =
        FMatchmakingEngineRequestComplete::CreateLambda([this, InstanceIdx, Instances, OnDone](
                                                            const FString &TaskId,
                                                            const FMatchmakingEngineRequest &,
                                                            FMatchmakingEngineResponse Response) {
            this->TestFalse(
                TEXT("Did not expect to get OnComplete for cancellation test!"),
                InstanceIdx == 5 || InstanceIdx == 6);
            OnDone.ExecuteIfBound();
        });
    Request.OnProgress = FMatchmakingEngineRequestProgress::CreateLambda(
        [InstanceIdx,
         this](const FString &TaskId, const FMatchmakingEngineRequest &, FMatchmakingEngineProgressInfo ProgressInfo) {
            UE_LOG(
                LogMatchmakingTests,
                Verbose,
                TEXT("Instance(%d) matchmaking unit test got update from matchmaker: %s"),
                InstanceIdx,
                *ProgressInfo.StepName.ToString());

            if (InstanceIdx == 5 && ProgressInfo.StepName.ToString() == TEXT("FollowHost"))
            {
                UE_LOG(
                    LogMatchmakingTests,
                    Verbose,
                    TEXT("Instance(%d) is calling Cancel() to abandon matchmaking!"),
                    InstanceIdx,
                    *ProgressInfo.StepName.ToString());
                Engine->Cancel(this->GetRequestHandleForInstance(InstanceIdx));
            }
        });
    Request.OnCancelled = FMatchmakingEngineRequestCancelled::CreateLambda(
        [this, InstanceIdx, Instances, OnDone](const FString &TaskId, const FMatchmakingEngineRequest &) {
            this->TestTrue(
                TEXT("Did not expect to get OnComplete for cancellation test!"),
                InstanceIdx == 5 || InstanceIdx == 6);
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