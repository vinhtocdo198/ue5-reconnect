// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingCandidateEvaluatorDefault.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "./Openskill/Openskill.h"
#include "MatchmakingStateHash.h"
#include "Misc/AutomationTest.h"
#include "Misc/Optional.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

struct FNewTeamLayout
{
    TUserIdMap<FMatchmakingTeamId> TeamReassignments;
    TArray<int32> LocalSlotsEmpty;
    int32 TotalCapacity;
};

static TOptional<FNewTeamLayout> ComputeNewTeamLayout(
    const TArray<int32> &TeamCapacities,
    const TArray<int32> &TeamAssignedMemberCount,
    const TArray<TSharedRef<const FUniqueNetId>> &CandidateMemberIds,
    const TUserIdMap<FMatchmakingTeamId> &CandidateMemberTeams,
    EMatchmakingBalanceMode BalanceMode)
{
    // Compute initial values from local and candidate states.
    TArray<int32> LocalSlotsEmpty;
    TArray<int32> LocalSlotsFilled;
    TArray<int32> CandidateSlotsFilled;
    int TotalCapacity = 0;
    for (int i = 0; i < TeamCapacities.Num(); i++)
    {
        LocalSlotsEmpty.Add(TeamCapacities[i] - TeamAssignedMemberCount[i]);
        LocalSlotsFilled.Add(TeamAssignedMemberCount[i] /* Task.TeamManager->GetAssignedMemberCountOfTeam(i) */);
        CandidateSlotsFilled.Add(0);
        TotalCapacity += TeamCapacities[i];
    }
    TMultiMap<FMatchmakingTeamId, TSharedPtr<const FUniqueNetId>> CandidateTeamsToMembers;
    for (int32 i = 0; i < CandidateMemberIds.Num(); i++)
    {
        TSharedPtr<const FUniqueNetId> UserId = CandidateMemberIds[i];
        int64 Team = CandidateMemberTeams[*UserId];

        CandidateSlotsFilled[(int)Team]++;
        CandidateTeamsToMembers.Add((FMatchmakingTeamId)Team, UserId);
    }

    // Attempt to assign teams from the candidate lobby into our lobby.
    TUserIdMap<FMatchmakingTeamId> TeamReassignments;

    // Sort the candidate teams so that the team with the most members is first.
    TArray<TTuple<FMatchmakingTeamId, int32>> CandidateSlotsFilledTuples;
    for (FMatchmakingTeamId CandidateTeamId = 0; CandidateTeamId < CandidateSlotsFilled.Num(); CandidateTeamId++)
    {
        CandidateSlotsFilledTuples.Add(
            TTuple<FMatchmakingTeamId, int32>(CandidateTeamId, CandidateSlotsFilled[CandidateTeamId]));
    }
    CandidateSlotsFilledTuples.StableSort(
        [](const TTuple<FMatchmakingTeamId, int32> &A, const TTuple<FMatchmakingTeamId, int32> &B) {
            return !(A.Get<1>() < B.Get<1>());
        });

    // Iterate through each candidate team from largest to smallest.
    for (const auto &Tuple : CandidateSlotsFilledTuples)
    {
        FMatchmakingTeamId TargetLocalTeam = -1;
        int32 ClosestRemainingToZeroValue = 9999;

        // When in MaximizeTeamFill, we want to find the local team that would be "most filled" by this candidate
        // team. Since we're processing the largest candidate teams first, this means that we'll combine the "most
        // filled" local team with the candidate team with the "most members" that doesn't also go over the team
        // capacity.
        if (BalanceMode == EMatchmakingBalanceMode::MaximizeTeamFill)
        {
            for (FMatchmakingTeamId LocalTeamId = 0; LocalTeamId < LocalSlotsEmpty.Num(); LocalTeamId++)
            {
                int32 RemainingSlotsAfterPick = LocalSlotsEmpty[LocalTeamId] - Tuple.Get<1>();
                if (RemainingSlotsAfterPick < 0)
                {
                    // This would overfill the team; we can't pick it.
                    continue;
                }
                else if (RemainingSlotsAfterPick < ClosestRemainingToZeroValue)
                {
                    // Putting the candidate team into this local team is a better fit than the current selection.
                    TargetLocalTeam = LocalTeamId;
                    ClosestRemainingToZeroValue = RemainingSlotsAfterPick;
                }
            }
        }
        // When in MaximizeBalance, we want to find the local team that has the least number of members that can
        // still fit this candidate team. Since we're processing the largest candidate teams first, that means we'll
        // take the emptiest local team and fill it with the most members. By process of elimination, this means
        // we'll be avoiding filling "more filled" local teams with as many members, since the remaining candidate
        // teams will always have equal to or less members than the earlier candidate teams we processed.
        else
        {
            for (FMatchmakingTeamId LocalTeamId = 0; LocalTeamId < LocalSlotsEmpty.Num(); LocalTeamId++)
            {
                int32 RemainingSlotsAfterPick = LocalSlotsEmpty[LocalTeamId] - Tuple.Get<1>();
                if (RemainingSlotsAfterPick < 0)
                {
                    // This would overfill the team; we can't pick it.
                    continue;
                }
                else if (LocalSlotsFilled[LocalTeamId] < ClosestRemainingToZeroValue)
                {
                    // Putting the candidate team into this local team is a better fit than the current selection.
                    TargetLocalTeam = LocalTeamId;
                    ClosestRemainingToZeroValue = LocalSlotsFilled[LocalTeamId];
                }
            }
        }

        if (TargetLocalTeam == -1)
        {
            // This candidate team can not fit into any local team. This candidate is incompatible.
            return TOptional<FNewTeamLayout>();
        }
        else
        {
            // All the members on this candidate team can move to the target team.
            TArray<TSharedPtr<const FUniqueNetId>> CandidateTeamMembers;
            CandidateTeamsToMembers.MultiFind(Tuple.Get<0>(), CandidateTeamMembers, false);
            CandidateTeamsToMembers.Remove(Tuple.Get<0>());
            CandidateSlotsFilled[Tuple.Get<0>()] = 0;
            for (const auto &UserId : CandidateTeamMembers)
            {
                TeamReassignments.Add(*UserId, TargetLocalTeam);
                LocalSlotsEmpty[TargetLocalTeam]--;
                LocalSlotsFilled[TargetLocalTeam]++;
            }
        }
    }

    // We successfully filled all teams.
    return FNewTeamLayout{TeamReassignments, LocalSlotsEmpty, TotalCapacity};
}

FMatchmakingSuitableMatchResult FMatchmakingCandidateEvaluatorDefault::EvaluateCandidate(
    FMatchmakingEngineTask &Task,
    const FMatchmakingCandidateLobby &Candidate,
    double SecondsRemaining,
    double IterationMaximumSecondsRemaining)
{
    if (auto Pins = Task.PinAndContinue())
    {
        FMatchmakingSuitableMatchResult ErrorResult(Candidate.LobbyId, Candidate.StateHash);
        ErrorResult.Suitable = false;

        // Ensure the candidate doesn't have any members who are already in our team manager (this can happen
        // if the same user has entered matchmaking twice on two different machines).
        for (const auto &MemberId : Candidate.MemberIds)
        {
            if (Task.TeamManager->GetAssignedTeam(*MemberId) != EMatchmakingTeamId_None)
            {
                FMatchmakingSuitableMatchResult Result(Candidate.LobbyId, Candidate.StateHash);
                Result.SuitabilityScore = -1;
                Result.Suitable = false;
                Result.UnsuitabilityReason = "shares an existing member with us";
                return Result;
            }
        }

        // Attempt to perform team reassignment based on balance mode.
        TArray<int32> TeamAssignedMemberCount;
        for (FMatchmakingTeamId LocalTeamId = 0; LocalTeamId < Task.Request.HostConfiguration->TeamCapacities.Num();
             LocalTeamId++)
        {
            TeamAssignedMemberCount.Add(Task.TeamManager->GetAssignedMemberCountOfTeam(LocalTeamId));
        }
        auto NewTeamLayout = ComputeNewTeamLayout(
            Task.Request.HostConfiguration->TeamCapacities,
            TeamAssignedMemberCount,
            Candidate.MemberIds,
            Candidate.MemberTeams,
            Task.Request.HostConfiguration->BalanceMode);
        if (!NewTeamLayout.IsSet())
        {
            FMatchmakingSuitableMatchResult Result(Candidate.LobbyId, Candidate.StateHash);
            Result.SuitabilityScore = -1;
            Result.Suitable = false;
            Result.UnsuitabilityReason = TEXT("unable to fit into lobby");
            return Result;
        }

        // Check if we need to evaluate skill-based matchmaking.
        double SkillBalanceReward = 0.0;
        if (Candidate.bSkillEnabled)
        {
            // Compute the threshold context for evaluating the maximum balance differential.
            FMatchmakingSkillThresholdFunctionContext ThresholdContext = {};
            ThresholdContext.SecondsRemaining = (float)SecondsRemaining;
            ThresholdContext.IterationMaximumSecondsRemaining = (float)IterationMaximumSecondsRemaining;

            // Get the maximum balance differential.
            double MaximumBalanceDifferential;
            if (Task.Request.HostConfiguration->SkillThresholdFunction.IsBound())
            {
                MaximumBalanceDifferential =
                    Task.Request.HostConfiguration->SkillThresholdFunction.Execute(ThresholdContext);
            }
            else
            {
                // Default balance differential implementation.
                MaximumBalanceDifferential =
                    10.0 +
                    (1.0 - FMath::Max(
                               0.0,
                               (static_cast<double>(ThresholdContext.SecondsRemaining) /
                                (static_cast<double>(ThresholdContext.IterationMaximumSecondsRemaining) - 5.0)))) *
                        90.0;
            }

            // Constrain balance differential.
            if (MaximumBalanceDifferential < 1.0)
            {
                UE_LOG(
                    LogMatchmaking,
                    Warning,
                    TEXT("The returned maximum balance differential for skill-based matchmaking was less than 1.0. It "
                         "was automatically increased to 1.0, as lower thresholds are unlikely to result in successful "
                         "matches."))
                MaximumBalanceDifferential = 1.0;
            }

            // If the balance differential is 100.0, then we accept any candidates and don't do skill-based
            // matchmaking evaluation at all.
            if (MaximumBalanceDifferential >= 100.0)
            {
                // Skip skill-based matchmaking evaluation.
            }
            else
            {
                // Compute what each team would look like (using the team manager and pending team reassignments).
                TArray<TArray<TSharedPtr<const FUniqueNetId>>> TheoreticalTeamLayout =
                    Task.TeamManager->ComputeTheoreticalTeamLayout(NewTeamLayout->TeamReassignments);
                TArray<FOpenskillTeam> OpenskillTeams;
                for (const auto &TheoreticalTeam : TheoreticalTeamLayout)
                {
                    TArray<FOpenskillRating> Players;
                    for (const auto &TheoreticalUserId : TheoreticalTeam)
                    {
                        if (Candidate.MemberSkillRating.Contains(*TheoreticalUserId))
                        {
                            // New member from candidate.
                            Players.Add(Candidate.MemberSkillRating[*TheoreticalUserId]);
                        }
                        else
                        {
                            // Existing member of lobby, need to get existing skill rating from team manager.
                            Players.Add(Task.TeamManager->GetUserOpenskillRating(*TheoreticalUserId));
                        }
                    }

                    // Skip empty teams.
                    if (Players.Num() != 0)
                    {
                        OpenskillTeams.Add(FOpenskillTeam(Players));
                    }
                }

                // If we only have a single team, then we can't evaluate skills (this happens if the
                // match is configured for MaximizeTeamFill and we haven't started loading players
                // onto the second team yet).
                if (OpenskillTeams.Num() >= 2)
                {
                    // Compute likelihood of each team winning.
                    TArray<double> BalanceProbabilities = FOpenskill::PredictWinners(OpenskillTeams);
                    double MinProbability = 100.0;
                    double MaxProbability = 0.0;
                    for (const double &Probability : BalanceProbabilities)
                    {
                        if (Probability * 100.0 > MaxProbability)
                        {
                            MaxProbability = Probability * 100.0;
                        }
                        if (Probability * 100.0 < MinProbability)
                        {
                            MinProbability = Probability * 100.0;
                        }
                    }

                    // If the balance differential is too great, this candidate can not be selected.
                    double BalanceDifferential = FMath::Abs(MaxProbability - MinProbability);
                    if (BalanceDifferential > MaximumBalanceDifferential)
                    {
                        FMatchmakingSuitableMatchResult Result(Candidate.LobbyId, Candidate.StateHash);
                        Result.SuitabilityScore = -1;
                        Result.Suitable = false;
                        Result.UnsuitabilityReason = FString::Printf(
                            TEXT("skill difference too great (%f > %f)"),
                            BalanceDifferential,
                            MaximumBalanceDifferential);
                        return Result;
                    }

                    // Reward the candidate based on how close the balance is.
                    double BalanceDifferentialPercentage = BalanceDifferential / MaximumBalanceDifferential;
                    SkillBalanceReward = 100.0 * (1 - BalanceDifferentialPercentage);
                }
            }
        }

        // Otherwise, this is a compatible match.
        FMatchmakingSuitableMatchResult Result(Candidate.LobbyId, Candidate.StateHash);
        Result.Suitable = true;
        Result.MemberSkillRating = Candidate.MemberSkillRating;
        if (Task.Request.HostConfiguration->BalanceMode == EMatchmakingBalanceMode::MaximizeBalance)
        {
            // Score higher for the more slots we fill overall.
            Result.SuitabilityScore = 0;
            for (auto MissingSlots : NewTeamLayout->LocalSlotsEmpty)
            {
                Result.SuitabilityScore += MissingSlots;
            }
            Result.SuitabilityScore = (int)(((NewTeamLayout->TotalCapacity - Result.SuitabilityScore) * 200LL) +
                                            FMath::TruncToInt(SkillBalanceReward));
        }
        else /* BalanceMode == MaximizeTeamFill */
        {
            // Score higher the closer we get to full teams.
            Result.SuitabilityScore = 0;
            int32 MinimumNumberOfEmptySlotsOnATeam = 0;
            int32 TotalSlots = 0;
            for (int i = 0; i < Task.Request.HostConfiguration->TeamCapacities.Num(); i++)
            {
                int32 TeamCapacity = Task.Request.HostConfiguration->TeamCapacities[i];
                if (MinimumNumberOfEmptySlotsOnATeam < TeamCapacity)
                {
                    MinimumNumberOfEmptySlotsOnATeam = TeamCapacity;
                }
                TotalSlots += TeamCapacity;
            }
            for (const auto &MissingSlots : NewTeamLayout->LocalSlotsEmpty)
            {
                if (MissingSlots < MinimumNumberOfEmptySlotsOnATeam)
                {
                    MinimumNumberOfEmptySlotsOnATeam = MissingSlots;
                }
            }
            Result.SuitabilityScore = (int)(((TotalSlots - MinimumNumberOfEmptySlotsOnATeam) * 100LL) +
                                            FMath::TruncToInt(SkillBalanceReward));
        }
        Result.TeamReassignments = NewTeamLayout->TeamReassignments;
        return Result;
    }

    FMatchmakingSuitableMatchResult Result(Candidate.LobbyId, Candidate.StateHash);
    Result.Suitable = false;
    Result.UnsuitabilityReason = "no lobby or party interface available";
    return Result;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMatchmakingCandidateEvaluatorTest,
    "Redpoint.Matchmaking.CandidateEvaluator",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMatchmakingCandidateEvaluatorTest::RunTest(const FString &Parameters)
{
    {
        auto MemberId1 = FUniqueNetIdString::Create(TEXT("1"), TEXT("Test"));
        auto CandidateTeams = TUserIdMap<FMatchmakingTeamId>();
        CandidateTeams.Add(*MemberId1, 0);
        auto NewTeamLayout = ComputeNewTeamLayout(
            TArray<int32>{5, 5},
            TArray<int32>{1, 2},
            TArray<TSharedRef<const FUniqueNetId>>{MemberId1},
            CandidateTeams,
            EMatchmakingBalanceMode::MaximizeBalance);
        if (this->TestTrue(TEXT("Candidate was compatible"), NewTeamLayout.IsSet()))
        {
            this->TestEqual(TEXT("Local slots empty [0]"), NewTeamLayout->LocalSlotsEmpty[0], 3);
            this->TestEqual(TEXT("Local slots empty [1]"), NewTeamLayout->LocalSlotsEmpty[1], 3);
            if (this->TestTrue(
                    TEXT("Candidate reassigns member 1"),
                    NewTeamLayout->TeamReassignments.Contains(*MemberId1)))
            {
                this->TestEqual(
                    TEXT("Candidate member 1 assigned to team 0"),
                    NewTeamLayout->TeamReassignments[*MemberId1],
                    0);
            }
        }
    }

    {
        auto MemberId1 = FUniqueNetIdString::Create(TEXT("1"), TEXT("Test"));
        auto CandidateTeams = TUserIdMap<FMatchmakingTeamId>();
        CandidateTeams.Add(*MemberId1, 0);
        auto NewTeamLayout = ComputeNewTeamLayout(
            TArray<int32>{5, 5},
            TArray<int32>{1, 2},
            TArray<TSharedRef<const FUniqueNetId>>{MemberId1},
            CandidateTeams,
            EMatchmakingBalanceMode::MaximizeTeamFill);
        if (this->TestTrue(TEXT("Candidate was compatible"), NewTeamLayout.IsSet()))
        {
            this->TestEqual(TEXT("Local slots empty [0]"), NewTeamLayout->LocalSlotsEmpty[0], 4);
            this->TestEqual(TEXT("Local slots empty [1]"), NewTeamLayout->LocalSlotsEmpty[1], 2);
            if (this->TestTrue(
                    TEXT("Candidate reassigns member 1"),
                    NewTeamLayout->TeamReassignments.Contains(*MemberId1)))
            {
                this->TestEqual(
                    TEXT("Candidate member 1 assigned to team 1"),
                    NewTeamLayout->TeamReassignments[*MemberId1],
                    1);
            }
        }
    }

    {
        auto MemberId1 = FUniqueNetIdString::Create(TEXT("1"), TEXT("Test"));
        auto MemberId2 = FUniqueNetIdString::Create(TEXT("2"), TEXT("Test"));
        auto CandidateTeams = TUserIdMap<FMatchmakingTeamId>();
        CandidateTeams.Add(*MemberId1, 0);
        CandidateTeams.Add(*MemberId2, 1);
        auto NewTeamLayout = ComputeNewTeamLayout(
            TArray<int32>{5, 5},
            TArray<int32>{1, 2},
            TArray<TSharedRef<const FUniqueNetId>>{MemberId1, MemberId2},
            CandidateTeams,
            EMatchmakingBalanceMode::MaximizeTeamFill);
        if (this->TestTrue(TEXT("Candidate was compatible"), NewTeamLayout.IsSet()))
        {
            this->TestEqual(TEXT("Local slots empty [0]"), NewTeamLayout->LocalSlotsEmpty[0], 4);
            this->TestEqual(TEXT("Local slots empty [1]"), NewTeamLayout->LocalSlotsEmpty[1], 1);
            if (this->TestTrue(
                    TEXT("Candidate reassigns member 1"),
                    NewTeamLayout->TeamReassignments.Contains(*MemberId1)))
            {
                this->TestEqual(
                    TEXT("Candidate member 1 assigned to team 1"),
                    NewTeamLayout->TeamReassignments[*MemberId1],
                    1);
            }
            if (this->TestTrue(
                    TEXT("Candidate reassigns member 2"),
                    NewTeamLayout->TeamReassignments.Contains(*MemberId2)))
            {
                this->TestEqual(
                    TEXT("Candidate member 2 assigned to team 1"),
                    NewTeamLayout->TeamReassignments[*MemberId2],
                    1);
            }
        }
    }

    return true;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()