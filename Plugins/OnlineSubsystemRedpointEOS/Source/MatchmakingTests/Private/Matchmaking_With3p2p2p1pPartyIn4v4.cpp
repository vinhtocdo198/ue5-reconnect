// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

IMPLEMENT_MATCHMAKING_PARTY_AUTOMATION_TEST(
    FMatchmaking_With3p2p2p1pPartyIn4v4,
    "Redpoint.Matchmaking.With3p2p2p1pPartyIn4v4",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter,
    "3:2:2:1");

FMatchmakingEngineRequestHandle FMatchmaking_With3p2p2p1pPartyIn4v4::QueueRequestForInstance(
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
            UE_LOG(LogMatchmakingTests, Verbose, TEXT("Instance(%d) matchmaking unit test is complete"), InstanceIdx);

            ASSERT_MM_SLOT_USER(0, 0, 0);
            ASSERT_MM_SLOT_USER(0, 1, 1);
            ASSERT_MM_SLOT_USER(0, 2, 2);
            ASSERT_MM_SLOT_USER(0, 3, 7);

            ASSERT_MM_SLOT_USER(1, 0, 6);
            ASSERT_MM_SLOT_USER(1, 1, 5);
            ASSERT_MM_SLOT_USER(1, 2, 4);
            ASSERT_MM_SLOT_USER(1, 3, 3);

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
        [this, Instances, OnDone](const FString &TaskId, const FMatchmakingEngineRequest &) {
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