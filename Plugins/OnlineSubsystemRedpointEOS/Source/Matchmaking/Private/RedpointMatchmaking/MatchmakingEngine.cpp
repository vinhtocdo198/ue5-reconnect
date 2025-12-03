// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/MatchmakingEngine.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingPerformSearch.h"
#include "RedpointMatchmaking/MatchmakingEngineImpl.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/Openskill/Openskill.h"
#include "RedpointMatchmaking/Openskill/OpenskillUtils.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<IMatchmakingEngine> IMatchmakingEngine::Instance = nullptr;

TSharedPtr<IMatchmakingEngine> IMatchmakingEngine::Get()
{
    if (Instance == nullptr)
    {
        auto ImplInstance = MakeShared<FMatchmakingEngineImpl>(false);
        ImplInstance->RegisterSteps();
        Instance = ImplInstance;
    }
    return Instance;
}

TSharedPtr<IMatchmakingEngine> IMatchmakingEngine::ForciblyCreateInstanceForUnitTests()
{
    auto ImplInstance = MakeShared<FMatchmakingEngineImpl>(true);
    ImplInstance->RegisterSteps();
    return ImplInstance;
}

void IMatchmakingEngine::SubmitMatchResults(
    const TSharedRef<const FUniqueNetId> &LocalUserId,
    const TSharedRef<class IOnlineStats, ESPMode::ThreadSafe> &InStatsInterface,
    const FString &SkillStatPrefix,
    const FMatchmakingEngineResponse &OriginalMatchResponse,
    const TArray<double> &TeamScores,
    const FMatchmakingEngineSubmitComplete &OnComplete)
{
    if (TeamScores.Num() != OriginalMatchResponse.Teams.Num())
    {
        OnComplete.ExecuteIfBound(OnlineMatchmaking::Errors::IncorrectNumberOfTeamScores());
        return;
    }

    TArray<FString> StatNames;
    StatNames.Add(FString::Printf(TEXT("%s_mu"), *SkillStatPrefix));
    StatNames.Add(FString::Printf(TEXT("%s_sigma"), *SkillStatPrefix));

    TArray<TSharedRef<const FUniqueNetId>> TargetUserIds;
    TUserIdSet UsersToLoadStatsFor;
    for (const auto &Team : OriginalMatchResponse.Teams)
    {
        for (const auto &Slot : Team.Slots)
        {
            if (Slot.Type == EMatchmakingEngineResponseTeamSlotType::User)
            {
                TargetUserIds.Add(Slot.UserId.ToSharedRef());
                UsersToLoadStatsFor.Add(*Slot.UserId);
            }
        }
    }

    // Load the current stats. We don't use cached values since their stats
    // might have updated in the background in some weird concurrent scenarios.
    InStatsInterface->QueryStats(
        LocalUserId,
        TargetUserIds,
        StatNames,
        FOnlineStatsQueryUsersStatsComplete::CreateLambda(
            [WeakStats = TWeakPtr<IOnlineStats, ESPMode::ThreadSafe>(InStatsInterface),
             LocalUserId,
             OnComplete,
             SkillStatPrefix,
             OriginalMatchResponse,
             TeamScores,
             UsersToLoadStatsFor](
                const FOnlineError &Error,
                const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult) {
                if (!Error.bSucceeded || Error.ErrorCode.EndsWith(".timed_out"))
                {
                    OnComplete.ExecuteIfBound(Error);
                    return;
                }

                auto Stats = WeakStats.Pin();
                if (!Stats.IsValid())
                {
                    OnComplete.ExecuteIfBound(OnlineMatchmaking::Errors::MissingStatsImplementation());
                    return;
                }

                TUserIdMap<FOpenskillRating> LoadedRatings =
                    FMatchmakingAlgorithmStepSearchingPerformSearch::DeserializeOpenskillStats(
                        SkillStatPrefix,
                        UsersStatsResult,
                        UsersToLoadStatsFor);

                // Load teams from the original matchmaking response.
                TArray<FOpenskillTeam> OpenskillTeams;
                for (const auto &Team : OriginalMatchResponse.Teams)
                {
                    TArray<FOpenskillRating> Players;
                    for (const auto &Slot : Team.Slots)
                    {
                        if (Slot.Type == EMatchmakingEngineResponseTeamSlotType::User)
                        {
                            Players.Add(LoadedRatings[*Slot.UserId]);
                        }
                    }
                    OpenskillTeams.Add(FOpenskillTeam(Players));
                }

                // Rank team indexes by score.
                TArray<double> TeamRanks;
                for (int32 i = 0; i < TeamScores.Num(); i++)
                {
                    TeamRanks.Add(-TeamScores[i]); // Rank is 0=winner while score is highest value=winner.
                }
                TArray<int32> OriginalIndexes;
                TArray<FOpenskillTeam> SortedOpenskillTeams;
                TArray<double> SortedTeamRanks;
                Openskill::Unwind<FOpenskillTeam, double>(
                    TeamRanks,
                    OpenskillTeams,
                    OriginalIndexes,
                    SortedOpenskillTeams,
                    SortedTeamRanks);

                auto Results = FOpenskill::Rate(SortedOpenskillTeams, SortedTeamRanks);

                TUserIdMap<FOpenskillRating> NewRatings;
                for (int32 i = 0; i < OriginalIndexes.Num(); i++)
                {
                    const auto &Team = OriginalMatchResponse.Teams[OriginalIndexes[i]];
                    int32 j = 0;
                    for (const auto &Slot : Team.Slots)
                    {
                        if (Slot.Type == EMatchmakingEngineResponseTeamSlotType::User)
                        {
                            NewRatings.Add(*Slot.UserId, Results[i].GetPlayerRatings()[j]);
                            j++;
                        }
                    }
                }

                // Submit the new ratings to the backend.
                TArray<FOnlineStatsUserUpdatedStats> UpdatedStats;
                for (const auto &KV : NewRatings)
                {
                    FOnlineStatsUserUpdatedStats NewStats(KV.Key);
                    NewStats.Stats.Add(
                        FString::Printf(TEXT("%s_mu"), *SkillStatPrefix),
                        FOnlineStatUpdate(KV.Value.GetMu(), FOnlineStatUpdate::EOnlineStatModificationType::Set));
                    NewStats.Stats.Add(
                        FString::Printf(TEXT("%s_sigma"), *SkillStatPrefix),
                        FOnlineStatUpdate(KV.Value.GetSigma(), FOnlineStatUpdate::EOnlineStatModificationType::Set));
                    UpdatedStats.Add(NewStats);
                }
                Stats->UpdateStats(
                    LocalUserId,
                    UpdatedStats,
                    FOnlineStatsUpdateStatsComplete::CreateLambda([OnComplete](const FOnlineError &UpdateError) {
                        if (!UpdateError.bSucceeded || UpdateError.ErrorCode.EndsWith(".timed_out"))
                        {
                            OnComplete.ExecuteIfBound(UpdateError);
                            return;
                        }

                        OnComplete.ExecuteIfBound(FOnlineError::Success());
                    }));
            }));
}

#endif

REDPOINT_EOS_CODE_GUARD_END()