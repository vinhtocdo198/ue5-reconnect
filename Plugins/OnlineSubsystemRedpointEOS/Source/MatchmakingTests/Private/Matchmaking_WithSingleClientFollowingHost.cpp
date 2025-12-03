// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

IMPLEMENT_MATCHMAKING_PARTY_AUTOMATION_TEST(
    FMatchmaking_WithSingleClientFollowingHost,
    "Redpoint.Matchmaking.WithSingleClientFollowingHost",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter,
    "2");

FMatchmakingEngineRequestHandle FMatchmaking_WithSingleClientFollowingHost::QueueRequestForInstance(
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
        HostConfiguration->TeamCapacities.Add(2);
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
            this->TestEqual(
                FString::Printf(TEXT("Instance(%d) matchmaking user in team 0 slot 0 is user 0"), InstanceIdx),
                *Response.Teams[0].Slots[0].UserId,
                *Instances[0].UserId);
            this->TestEqual(
                FString::Printf(TEXT("Instance(%d) matchmaking user in team 0 slot 1 is user 1"), InstanceIdx),
                *Response.Teams[0].Slots[1].UserId,
                *Instances[1].UserId);
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