// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingPerformSearch.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingCandidateEvaluator.h"
#include "../MatchmakingStateHash.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatch.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearching.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingSendFollowRequest.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerReceiveFollowRequest.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateSearchingPerformSearch> FMatchmakingAlgorithmStepSearchingPerformSearch::
    CreateNewState()
{
    auto State = MakeShared<FMatchmakingAlgorithmStepStateSearchingPerformSearch>();
    State->bInterruptedWithFollowRequest = false;
    State->CompletionTime = FDateTime::MinValue();
    return State;
}

TSharedPtr<struct FMatchmakingCandidateLobby> FMatchmakingAlgorithmStepSearchingPerformSearch::
    LoadCandidateLobbyBasicInfo(
        FMatchmakingEngineTask &Task,
        const FOnlineLobbyId &LobbyId,
        FString &UnsuitabilityReason,
        FString &StateHash)
{
    if (auto Pins = Task.PinAndContinue())
    {
        StateHash = FMatchmakingStateHash::ComputeStateHashForLobby(*Pins->OnlineLobby, *Task.Request.UserId, LobbyId);

        if (StateHash.IsEmpty())
        {
            UnsuitabilityReason = "can't compute state hash";
            return nullptr;
        }

        if (Task.ExcludedLobbyIds.Contains(LobbyId))
        {
            UnsuitabilityReason = "excluded by previous search";
            return nullptr;
        }

        int32 MemberCount;
        if (!Pins->OnlineLobby->GetMemberCount(*Task.Request.UserId, LobbyId, MemberCount))
        {
            UnsuitabilityReason = "unable to get member count";
            return nullptr;
        }

        auto Candidate = MakeShared<FMatchmakingCandidateLobby>(LobbyId, StateHash);

        for (int32 i = 0; i < MemberCount; i++)
        {
            TSharedPtr<const FUniqueNetId> UserId;
            if (!Pins->OnlineLobby->GetMemberUserId(*Task.Request.UserId, LobbyId, i, UserId))
            {
                UnsuitabilityReason = FString::Printf(TEXT("unable to get user ID for member %d"), i);
                return nullptr;
            }

            FVariantData TeamValue;
            FString TeamKey = FString::Printf(TEXT("%s.%s"), *UserId->ToString(), MatchmakingMemberMetadataTeam);
            if (!Pins->OnlineLobby->GetLobbyMetadataValue(*Task.Request.UserId, LobbyId, TeamKey, TeamValue) ||
                TeamValue.GetType() != EOnlineKeyValuePairDataType::Int64)
            {
                UnsuitabilityReason =
                    FString::Printf(TEXT("unable to get team number for member %d (user %s)"), i, *UserId->ToString());
                return nullptr;
            }

            int64 Team;
            TeamValue.GetValue(Team);

            Candidate->MemberIds.Add(UserId.ToSharedRef());
            Candidate->MemberTeams.Add(*UserId, (FMatchmakingTeamId)Team);
        }

        return Candidate;
    }

    UnsuitabilityReason = "missing required online subsystem interfaces";
    return nullptr;
}

void FMatchmakingAlgorithmStepSearchingPerformSearch::OnSearchComplete(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    const TArray<TSharedRef<const FOnlineLobbyId>> &Lobbies,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    auto State = this->GetState(*Task);
    if (State->bInterruptedWithFollowRequest)
    {
        // We are no longer the current step.
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_PERFORM_SEARCH))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (!Error.bSucceeded)
        {
            if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
            {
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sTimed out while searching for lobbies, scheduling another search"),
                    *this->GetLogPrefix(*Task));
                EMIT_TASK_COUNTER_TRACE(Task);
                this->MoveToStep(*Task, MMSTEP_SEARCHING, false);
                return;
            }

            this->RestartableError(
                *Task,
                FString::Printf(TEXT("Unable to search for lobbies: %s"), *Error.ToLogString()));
            return;
        }

        TArray<FMatchmakingSuitableMatchResult> AllCandidates;
        TArray<TSharedPtr<FMatchmakingCandidateLobby>> CandidateLobbies;
        TUserIdSet UsersToLoadStatsFor;
        for (int32 i = 0; i < Lobbies.Num(); i++)
        {
            FString UnsuitabilityReason;
            FString StateHash;
            TSharedPtr<FMatchmakingCandidateLobby> CandidateLobby =
                this->LoadCandidateLobbyBasicInfo(*Task, *(Lobbies[i]), UnsuitabilityReason, StateHash);
            if (CandidateLobby.IsValid())
            {
                if (!Task->Request.HostConfiguration->SkillStatPrefix.IsEmpty() && Pins->OnlineStats.IsValid())
                {
                    for (const auto &MemberId : CandidateLobby->MemberIds)
                    {
                        UsersToLoadStatsFor.Add(*MemberId);
                    }
                    CandidateLobby->bSkillEnabled = true;
                }

                CandidateLobbies.Add(CandidateLobby);
            }
            else
            {
                // Add the error result to the "all candidates" list.
                FMatchmakingSuitableMatchResult ErrorResult(Lobbies[i], StateHash);
                ErrorResult.Suitable = false;
                ErrorResult.UnsuitabilityReason = UnsuitabilityReason;
                AllCandidates.Add(ErrorResult);
            }
        }

        if (!Task->Request.HostConfiguration->SkillStatPrefix.IsEmpty() && Pins->OnlineStats.IsValid() &&
            UsersToLoadStatsFor.Num() > 0)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sNeed to load stats for %d users for skill-based matchmaking"),
                *this->GetLogPrefix(*Task),
                UsersToLoadStatsFor.Num());

            TArray<FString> StatNames;
            StatNames.Add(FString::Printf(TEXT("%s_mu"), *Task->Request.HostConfiguration->SkillStatPrefix));
            StatNames.Add(FString::Printf(TEXT("%s_sigma"), *Task->Request.HostConfiguration->SkillStatPrefix));
            TArray<TSharedRef<const FUniqueNetId>> TargetUserIds;
            for (const auto &TargetUserId : UsersToLoadStatsFor)
            {
                TargetUserIds.Add(TargetUserId);
            }

            Pins->OnlineStats->QueryStats(
                UserId.AsShared(),
                TargetUserIds,
                StatNames,
                FOnlineStatsQueryUsersStatsComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingPerformSearch::OnStatsCallback,
                    AllCandidates,
                    CandidateLobbies,
                    UsersToLoadStatsFor,
                    TaskWk));
        }
        else
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sNo user stats to load, immediately evaluating search candidates..."),
                *this->GetLogPrefix(*Task));
            this->OnStatsLoaded(UserId, AllCandidates, CandidateLobbies, TaskWk);
        }
    }
}

void FMatchmakingAlgorithmStepSearchingPerformSearch::OnStatsCallback(
    const FOnlineError &Error,
    const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
    TArray<struct FMatchmakingSuitableMatchResult> AllCandidates,
    TArray<TSharedPtr<struct FMatchmakingCandidateLobby>> CandidateLobbies,
    TUserIdSet UsersToLoadStatsFor,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    auto State = this->GetState(*Task);
    if (State->bInterruptedWithFollowRequest)
    {
        // We are no longer the current step.
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_PERFORM_SEARCH))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (!Error.bSucceeded)
        {
            if (Error.ErrorCode.EndsWith(".timed_out"))
            {
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sTimed out while loading user skill stats, scheduling another search"),
                    *this->GetLogPrefix(*Task));
                EMIT_TASK_COUNTER_TRACE(Task);
                this->MoveToStep(*Task, MMSTEP_SEARCHING, false);
                return;
            }

            this->RestartableError(
                *Task,
                FString::Printf(TEXT("Unable to load user skill stats: %s"), *Error.ToLogString()));
            return;
        }

        TUserIdMap<FOpenskillRating> LoadedRatings =
            FMatchmakingAlgorithmStepSearchingPerformSearch::DeserializeOpenskillStats(
                Task->Request.HostConfiguration->SkillStatPrefix,
                UsersStatsResult,
                UsersToLoadStatsFor);

        // Now fill the stats in for the candidate lobbies.
        for (const auto &CandidateLobby : CandidateLobbies)
        {
            for (const auto &MemberId : CandidateLobby->MemberIds)
            {
                CandidateLobby->MemberSkillRating.Add(*MemberId, LoadedRatings[*MemberId]);
            }
        }

        // Continue post-search logic.
        this->OnStatsLoaded(*Task->Request.UserId, MoveTemp(AllCandidates), CandidateLobbies, TaskWk);
    }
}

TUserIdMap<FOpenskillRating> FMatchmakingAlgorithmStepSearchingPerformSearch::DeserializeOpenskillStats(
    const FString &SkillStatPrefix,
    const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
    const TUserIdSet &UsersToLoadStatsFor)
{
    FString KeyMu = FString::Printf(TEXT("%s_mu"), *SkillStatPrefix);
    FString KeySigma = FString::Printf(TEXT("%s_sigma"), *SkillStatPrefix);

    // Load the stats.
    TUserIdMap<FOpenskillRating> LoadedRatings;
    for (const auto &LoadedStat : UsersStatsResult)
    {
        if (!LoadedStat->Stats.Contains(KeyMu) || !LoadedStat->Stats.Contains(KeySigma))
        {
            LoadedRatings.Add(*LoadedStat->Account, FOpenskillRating());
            continue;
        }

        const FOnlineStatValue &StatMu = LoadedStat->Stats[KeyMu];
        const FOnlineStatValue &StatSigma = LoadedStat->Stats[KeySigma];

        double ValueMu = 0.0;
        double ValueSigma = 0.0;
        bool bDidLoad = true;

        switch (StatMu.GetType())
        {
        case EOnlineKeyValuePairDataType::Float:
            float FloatMu;
            StatMu.GetValue(FloatMu);
            ValueMu = static_cast<double>(FloatMu);
            break;
        case EOnlineKeyValuePairDataType::Double:
            StatMu.GetValue(ValueMu);
            break;
        default:
            bDidLoad = false;
            break;
        }
        switch (StatSigma.GetType())
        {
        case EOnlineKeyValuePairDataType::Float:
            float FloatSigma;
            StatSigma.GetValue(FloatSigma);
            ValueSigma = static_cast<double>(FloatSigma);
            break;
        case EOnlineKeyValuePairDataType::Double:
            StatSigma.GetValue(ValueSigma);
            break;
        default:
            bDidLoad = false;
            break;
        }
        if (!bDidLoad)
        {
            LoadedRatings.Add(*LoadedStat->Account, FOpenskillRating());
            continue;
        }

        LoadedRatings.Add(*LoadedStat->Account, FOpenskillRating(ValueMu, ValueSigma));
    }
    for (const auto &TargetUserId : UsersToLoadStatsFor)
    {
        if (!LoadedRatings.Contains(*TargetUserId))
        {
            LoadedRatings.Add(*TargetUserId, FOpenskillRating());
        }
    }
    return LoadedRatings;
}

void FMatchmakingAlgorithmStepSearchingPerformSearch::OnStatsLoaded(
    const FUniqueNetId &UserId,
    TArray<FMatchmakingSuitableMatchResult> AllCandidates,
    const TArray<TSharedPtr<FMatchmakingCandidateLobby>> &CandidateLobbies,
    const TWeakPtr<FMatchmakingEngineTask> &TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    auto State = this->GetState(*Task);
    if (State->bInterruptedWithFollowRequest)
    {
        // We are no longer the current step.
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_PERFORM_SEARCH))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        TArray<FMatchmakingSuitableMatchResult> SuitableCandidates;
        for (const auto &CandidateLobby : CandidateLobbies)
        {
            FMatchmakingSuitableMatchResult MatchResult = Task->CandidateEvaluator->EvaluateCandidate(
                *Task,
                *CandidateLobby,
                FMath::Max(0.0, (State->CompletionTime - FDateTime::UtcNow()).GetTotalSeconds()),
                Task->Request.HostConfiguration->MinimumWaitSecondsBaseline +
                    (Task->Request.HostConfiguration->MinimumWaitSecondsPerEmptySlot *
                     Task->TeamManager->GetUnassignedSlotCount()) -
                    (Task->Request.HostConfiguration->PenaltyTimeSecondsPerRejectedRequest *
                     Task->RejectedRequestsPenaltyCount));
            AllCandidates.Add(MatchResult);
            if (MatchResult.Suitable)
            {
                SuitableCandidates.Add(MatchResult);
            }
            else if (Task->Request.HostConfiguration->OnCandidateRejected.IsBound())
            {
                Task->Request.HostConfiguration->OnCandidateRejected.Execute(
                    Task->TaskId,
                    Task->Request,
                    CandidateLobby->LobbyId->ToString(),
                    MatchResult.UnsuitabilityReason);
                if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING_PERFORM_SEARCH))
                {
                    // If the OnCandidateRejected callback cancels the request, then we will no longer be searching.
                    return;
                }
            }
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sGot %d suitable candidates in this search"),
            *this->GetLogPrefix(*Task),
            SuitableCandidates.Num());

        if (SuitableCandidates.Num() == 0 && AllCandidates.Num() <= 5)
        {
            // Small sample, iterate through all candidates we got and output their unsuitability
            // reason to the log (helps with debugging).
            for (const auto &Candidate : AllCandidates)
            {
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sCandidate lobby %s was unsuitable because: %s"),
                    *this->GetLogPrefix(*Task),
                    *Candidate.CandidateLobbyId->ToString(),
                    *Candidate.UnsuitabilityReason);
            }
        }

        if (SuitableCandidates.Num() == 0)
        {
            if (State->CompletionTime > FDateTime::UtcNow() &&
                Task->Request.HostConfiguration->OnNoCandidates != EMatchmakingBehaviourOnNoCandidates::WaitUntilFull)
            {
                this->RescheduleOnNoCandidatesFound(Task.ToSharedRef());
                return;
            }

            // Determine if we're meeting the minimum member count for partial fill mode (even if we're not in partial
            // fill mode).
            bool bHasMinimumMemberCountOnAllTeams = true;
            for (int i = 0; i < Task->Request.HostConfiguration->TeamCapacities.Num(); i++)
            {
                if (Task->Request.HostConfiguration->TeamCapacities[i] >= 1 &&
                    Task->TeamManager->GetAssignedMemberCountOfTeam(i) <
                        Task->Request.HostConfiguration->MinimumTeamMemberCountForPartialFill)
                {
                    bHasMinimumMemberCountOnAllTeams = false;
                }
            }

            // We have no candidates during a search. What we do now is dependent on the matchmaking
            // configuration.
            switch (Task->Request.HostConfiguration->OnNoCandidates)
            {
            case EMatchmakingBehaviourOnNoCandidates::WaitUntilFull:
                this->OnTimeout_WaitUntilFull(Task.ToSharedRef());
                return;
            case EMatchmakingBehaviourOnNoCandidates::CompletePartiallyFilled:
                if (bHasMinimumMemberCountOnAllTeams)
                {
                    this->OnTimeout_CompletePartiallyFilled(Task.ToSharedRef());
                }
                else
                {
                    this->OnTimeout_WaitUntilFull(Task.ToSharedRef());
                }
                return;
            case EMatchmakingBehaviourOnNoCandidates::CompleteFillWithAI:
                this->OnTimeout_CompleteFillWithAI(Task.ToSharedRef());
                return;
            default:
                checkf(false, TEXT("Unknown matchmaking behaviour on timeout!"));
                return;
            }
        }

        // Reset the completion time so that the next time we start the search we will recalculate
        // a new expiry.
        State->CompletionTime = FDateTime::MinValue();
        Task->EstimatedTimeOfCompletion = State->CompletionTime;

        TSharedPtr<FMatchmakingSuitableMatchResult> BestResult;
        int BestLobbyScore = -1;

        for (auto SuitableCandidate : SuitableCandidates)
        {
            if (SuitableCandidate.SuitabilityScore > BestLobbyScore)
            {
                BestLobbyScore = SuitableCandidate.SuitabilityScore;
                BestResult = MakeShared<FMatchmakingSuitableMatchResult>(SuitableCandidate);
            }
        }

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_SEARCHING_SEND_FOLLOW_REQUEST, *BestResult);
    }
}

void FMatchmakingAlgorithmStepSearchingPerformSearch::RescheduleOnNoCandidatesFound(
    const TSharedRef<FMatchmakingEngineTask> &Task)
{
    auto State = this->GetState(*Task);
    if (State->bInterruptedWithFollowRequest)
    {
        // We are no longer the current step.
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sNo candidates, scheduling another search in 1 second (%d seconds left until "
             "automatic completion)"),
        *this->GetLogPrefix(*Task),
        FMath::CeilToInt((State->CompletionTime - FDateTime::UtcNow()).GetTotalSeconds()));

    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_SEARCHING, false);
}

void FMatchmakingAlgorithmStepSearchingPerformSearch::OnTimeout_WaitUntilFull(
    const TSharedRef<FMatchmakingEngineTask> &Task)
{
    auto State = this->GetState(*Task);
    if (State->bInterruptedWithFollowRequest)
    {
        // We are no longer the current step.
        return;
    }

    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sNo candidates, scheduling another search in 1 second (wait forever)"),
        *this->GetLogPrefix(*Task));

    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_SEARCHING, false);
}

void FMatchmakingAlgorithmStepSearchingPerformSearch::OnTimeout_CompletePartiallyFilled(
    const TSharedRef<FMatchmakingEngineTask> &Task)
{
    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sNo candidates, returning a match that is partially filled"),
        *this->GetLogPrefix(*Task));
    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH);
}

void FMatchmakingAlgorithmStepSearchingPerformSearch::OnTimeout_CompleteFillWithAI(
    const TSharedRef<FMatchmakingEngineTask> &Task)
{
    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sNo candidates, returning a match that is filled with AI"),
        *this->GetLogPrefix(*Task));
    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_COMPLETE_MATCH);
}

void FMatchmakingAlgorithmStepSearchingPerformSearch::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        check(!Task->TeamManager->HasPendingBatch());

        auto State = this->GetState(*Task);
        State->bInterruptedWithFollowRequest = false;
        if (State->CompletionTime == FDateTime::MinValue())
        {
            State->CompletionTime =
                FDateTime::UtcNow() + FTimespan::FromSeconds(
                                          Task->Request.HostConfiguration->MinimumWaitSecondsBaseline +
                                          (Task->Request.HostConfiguration->MinimumWaitSecondsPerEmptySlot *
                                           Task->TeamManager->GetUnassignedSlotCount()) -
                                          (Task->Request.HostConfiguration->PenaltyTimeSecondsPerRejectedRequest *
                                           Task->RejectedRequestsPenaltyCount));
            Task->EstimatedTimeOfCompletion = State->CompletionTime;
        }

        UE_LOG(LogMatchmaking, Verbose, TEXT("%sSearching for lobbies for matchmaking"), *this->GetLogPrefix(*Task));

        TSharedRef<FOnlineLobbySearchQuery> Query = MakeShared<FOnlineLobbySearchQuery>();
        Query->Filters.Add(FOnlineLobbySearchQueryFilter(
            MatchmakingMetadataLobbyType,
            MatchmakingLobbyTypeMatch,
            EOnlineLobbySearchQueryFilterComparator::Equal));
        Query->Filters.Add(FOnlineLobbySearchQueryFilter(
            MatchmakingMetadataLobbyStatus,
            MatchmakingStatusSearching,
            EOnlineLobbySearchQueryFilterComparator::Equal));
        Query->Filters.Add(FOnlineLobbySearchQueryFilter(
            MatchmakingMetadataLobbyRequestId,
            Task->Request.HostConfiguration->RequestId,
            EOnlineLobbySearchQueryFilterComparator::GreaterThan));
        Query->Filters.Add(FOnlineLobbySearchQueryFilter(
            MatchmakingMetadataQueueName,
            Task->Request.HostConfiguration->QueueName,
            EOnlineLobbySearchQueryFilterComparator::Equal));
        if (!Task->Request.HostConfiguration->SkillStatPrefix.IsEmpty())
        {
            Query->Filters.Add(FOnlineLobbySearchQueryFilter(
                MatchmakingMetadataLobbySkillOrdinal,
                Task->TeamManager->GetMatchAverageOpenskillOrdinal(),
                EOnlineLobbySearchQueryFilterComparator::Distance));
        }
        Query->Limit = 100;

        if (!Pins->OnlineLobby->Search(
                *Task->Request.UserId,
                *Query,
                FOnLobbySearchComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepSearchingPerformSearch::OnSearchComplete,
                    TWeakPtr<FMatchmakingEngineTask>(Task))))
        {
            this->RestartableError(*Task, TEXT("Unable to start search for lobbies"));
            return;
        }
    }
}

bool FMatchmakingAlgorithmStepSearchingPerformSearch::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST);
}

bool FMatchmakingAlgorithmStepSearchingPerformSearch::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST))
    {
        auto State = this->GetState(*Task);
        State->bInterruptedWithFollowRequest = true;
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sSearch operation interrupted by follow request for lobby %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());
        return true;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()