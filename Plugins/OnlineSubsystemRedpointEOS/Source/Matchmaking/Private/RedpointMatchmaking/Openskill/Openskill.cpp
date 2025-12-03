// Copyright June Rhodes. All Rights Reserved.

#include "./Openskill.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "./OpenskillConstants.h"
#include "./OpenskillStatistics.h"

#include "gcem.hpp"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

struct FOpenskillTeamRating
{
private:
    double Mu;
    double SigmaSquared;
    FOpenskillTeam Team;
    FOpenskillRanking Rank;

public:
    FOpenskillTeamRating(
        const double &InMu,
        const double &InSigmaSquared,
        const FOpenskillTeam &InTeam,
        const FOpenskillRanking &InRank)
        : Mu(InMu)
        , SigmaSquared(InSigmaSquared)
        , Team(InTeam)
        , Rank(InRank)
    {
    }
    FOpenskillTeamRating(const FOpenskillTeamRating &In) = default;
    FOpenskillTeamRating(FOpenskillTeamRating &&In) = default;
    ~FOpenskillTeamRating() = default;

    FORCEINLINE double GetMu() const
    {
        return this->Mu;
    }
    FORCEINLINE double GetSigmaSquared() const
    {
        return this->SigmaSquared;
    }
    FORCEINLINE const FOpenskillTeam &GetTeam() const
    {
        return this->Team;
    }
    FORCEINLINE FOpenskillRanking GetRank() const
    {
        return this->Rank;
    }
    FORCEINLINE double GetGamma(const double &C) const
    {
        return gcem::sqrt(this->SigmaSquared) / C;
    }
};

/**
 * This normalizes rankings like [4, 5, 7, 7, 8] into [0, 1, 2, 2, 4], which allows
 * us to handle draws in matches.
 */
static TArray<int32> Rankings(const TArray<double> &DoubleRankings)
{
    TArray<int32> OutRank;
    int32 s = 0;
    for (int32 i = 0; i < DoubleRankings.Num(); i++)
    {
        if (i > 0)
        {
            if (DoubleRankings[i - 1] < DoubleRankings[i])
            {
                s = i;
            }
        }
        OutRank.Add(s);
    }
    return OutRank;
}

static TArray<FOpenskillTeamRating> TeamRating(
    const TArray<FOpenskillTeam> &RankedTeams,
    const TArray<double> &InitTeamRankings = TArray<double>())
{
    // If the team rankings aren't set, default to indexes.
    TArray<double> TeamRankings = InitTeamRankings;
    if (TeamRankings.Num() == 0)
    {
        for (int32 i = 0; i < RankedTeams.Num(); i++)
        {
            TeamRankings.Add((double)i);
        }
    }

    checkf(TeamRankings.Num() == RankedTeams.Num(), TEXT("Team rankings num must equal ranked teams num"));
    TArray<int32> NormalizedRankings = Rankings(TeamRankings);
    TArray<FOpenskillTeamRating> TeamRatings;
    for (int32 Index = 0; Index < RankedTeams.Num(); Index++)
    {
        const FOpenskillTeam &Team = RankedTeams[Index];
        const TArray<FOpenskillRating> &Players = Team.GetPlayerRatings();
        checkf(Players.Num() > 0, TEXT("Expected at least one player on each team!"));
        double MuI = 0.0;
        double SigmaSquared = 0.0;
        for (int32 PlayerIndex = 0; PlayerIndex < Players.Num(); PlayerIndex++)
        {
            const FOpenskillRating &Player = Players[PlayerIndex];
            if (PlayerIndex == 0)
            {
                MuI = Player.GetMu();
                SigmaSquared = gcem::pow(Player.GetSigma(), 2.0);
            }
            else
            {
                MuI += Player.GetMu();
                SigmaSquared += gcem::pow(Player.GetSigma(), 2.0);
            }
        }
        TeamRatings.Add(FOpenskillTeamRating(MuI, SigmaSquared, Team, (FOpenskillRanking)NormalizedRankings[Index]));
    }
    return TeamRatings;
}

TArray<FOpenskillTeam> FOpenskill::Rate(const TArray<FOpenskillTeam> &OrderedTeams, const TArray<double> &TeamRankings)
{
    // Compute team ratings.
    TArray<FOpenskillTeamRating> TeamRatings = TeamRating(OrderedTeams, TeamRankings);

    // Pre-compute values.
    double C;
    TArray<double> SumQ;
    TArray<int32> A;
    {
        double CollectiveTeamSigma = 0.0;
        for (const FOpenskillTeamRating &Team : TeamRatings)
        {
            CollectiveTeamSigma += (Team.GetSigmaSquared() + Openskill::DefaultBetaSquared);
        }
        C = gcem::sqrt(CollectiveTeamSigma);
    }
    {
        TMap<int32, double> SumQTemp;
        for (int32 Index = 0; Index < TeamRatings.Num(); Index++)
        {
            const FOpenskillTeamRating &Team = TeamRatings[Index];
            double Temp = gcem::exp(Team.GetMu() / C);
            for (int32 QIndex = 0; QIndex < TeamRatings.Num(); QIndex++)
            {
                const FOpenskillTeamRating &QTeam = TeamRatings[QIndex];
                if (Team.GetRank() >= QTeam.GetRank())
                {
                    if (SumQTemp.Contains(QIndex))
                    {
                        SumQTemp.Add(QIndex, SumQTemp[QIndex] + Temp);
                    }
                    else
                    {
                        SumQTemp.Add(QIndex, Temp);
                    }
                }
            }
        }
        for (int32 Index = 0; Index < TeamRatings.Num(); Index++)
        {
            SumQ.Add(SumQTemp[Index]);
        }
    }
    {
        for (int32 I = 0; I < TeamRatings.Num(); I++)
        {
            const FOpenskillTeamRating &Team = TeamRatings[I];
            int32 Count = 0;
            for (const FOpenskillTeamRating &QTeam : TeamRatings)
            {
                if (QTeam.GetRank() == Team.GetRank())
                {
                    Count++;
                }
            }
            A.Add(Count);
        }
    }

    // Compute new ratings.
    TArray<FOpenskillTeam> Results;
    for (int32 I = 0; I < TeamRatings.Num(); I++)
    {
        const FOpenskillTeamRating &ITeamRating = TeamRatings[I];
        double Omega = 0.0;
        double Delta = 0.0;
        double IMuOverCE = gcem::exp(ITeamRating.GetMu() / C);

        for (int32 Q = 0; Q < TeamRatings.Num(); Q++)
        {
            const FOpenskillTeamRating &QTeamRating = TeamRatings[Q];
            double IMuOverCEOverSumQ = IMuOverCE / SumQ[Q];
            if (QTeamRating.GetRank() <= ITeamRating.GetRank())
            {
                Delta += (IMuOverCEOverSumQ * (1 - IMuOverCEOverSumQ) / A[Q]);
                if (Q == I)
                {
                    Omega += (1 - IMuOverCEOverSumQ) / A[Q];
                }
                else
                {
                    Omega -= IMuOverCEOverSumQ / A[Q];
                }
            }
        }

        Omega *= ITeamRating.GetSigmaSquared() / C;
        Delta *= ITeamRating.GetSigmaSquared() / gcem::pow(C, 2.0);

        double Gamma = ITeamRating.GetGamma(C);
        Delta *= Gamma;

        TArray<FOpenskillRating> IntermediateResultPerTeam;
        for (int32 J = 0; J < ITeamRating.GetTeam().GetPlayerRatings().Num(); J++)
        {
            const FOpenskillRating &JPlayer = ITeamRating.GetTeam().GetPlayerRatings()[J];
            double NewMu = JPlayer.GetMu();
            double NewSigma = JPlayer.GetSigma();
            NewMu += (gcem::pow(NewSigma, 2.0) / ITeamRating.GetSigmaSquared()) * Omega;
            NewSigma *= gcem::sqrt(
                gcem::max(1 - (gcem::pow(NewSigma, 2.0) / ITeamRating.GetSigmaSquared()) * Delta, DBL_EPSILON));
            IntermediateResultPerTeam.Add(FOpenskillRating(NewMu, NewSigma));
        }
        Results.Add(IntermediateResultPerTeam);
    }
    return Results;
}

TArray<double> FOpenskill::PredictWinners(const TArray<FOpenskillTeam> &Teams)
{
    checkf(Teams.Num() >= 2, TEXT("Must have at least two teams when calling PredictWinners"));

    int32 N = Teams.Num();
    double Denom = (N * (N - 1)) / 2.0;

    TArray<double> PairwiseProbabilities;
    for (int32 I = 0; I < Teams.Num(); I++)
    {
        for (int32 J = 0; J < Teams.Num(); J++)
        {
            if (I == J)
            {
                continue;
            }

            const FOpenskillTeam &TeamA = Teams[I];
            const FOpenskillTeam &TeamB = Teams[J];
            FOpenskillTeamRating TeamRatingA = TeamRating(TArray<FOpenskillTeam>{TeamA})[0];
            FOpenskillTeamRating TeamRatingB = TeamRating(TArray<FOpenskillTeam>{TeamB})[0];
            double MuA = TeamRatingA.GetMu();
            double SigmaSquaredA = TeamRatingA.GetSigmaSquared();
            double MuB = TeamRatingB.GetMu();
            double SigmaSquaredB = TeamRatingB.GetSigmaSquared();
            PairwiseProbabilities.Add(Openskill::Statistics::PhiMajor(
                (MuA - MuB) / gcem::sqrt(
                                  N * gcem::pow(Openskill::DefaultBeta, 2.0) + gcem::pow(SigmaSquaredA, 2.0) +
                                  gcem::pow(SigmaSquaredB, 2.0))));
        }
    }

    TArray<double> Results;
    for (int32 I = 0; I < PairwiseProbabilities.Num(); I += (N - 1))
    {
        double Sum = 0;
        for (int32 J = I; J < I + (N - 1); J++)
        {
            Sum += PairwiseProbabilities[J];
        }
        Results.Add(Sum / Denom);
    }
    check(Results.Num() == Teams.Num());
    return Results;
}

double FOpenskill::PredictDraw(const TArray<FOpenskillTeam> &Teams)
{
    checkf(Teams.Num() >= 2, TEXT("Must have at least two teams when calling PredictDraw"));

    int32 N = Teams.Num();
    int32 TotalPlayerCount = 0;
    for (const auto &Team : Teams)
    {
        TotalPlayerCount += Team.GetPlayerRatings().Num();
    }
    double DrawProbability = 1.0 / N;
    double DrawMargin = gcem::sqrt(TotalPlayerCount) * Openskill::DefaultBeta *
                        Openskill::Statistics::PhiMajorInverse((1 + DrawProbability) / 2);

    double PairwiseProbabilitiesSum = 0;
    for (int32 I = 0; I < Teams.Num(); I++)
    {
        for (int32 J = 0; J < Teams.Num(); J++)
        {
            if (I == J)
            {
                continue;
            }

            const FOpenskillTeam &TeamA = Teams[I];
            const FOpenskillTeam &TeamB = Teams[J];
            FOpenskillTeamRating TeamRatingA = TeamRating(TArray<FOpenskillTeam>{TeamA})[0];
            FOpenskillTeamRating TeamRatingB = TeamRating(TArray<FOpenskillTeam>{TeamB})[0];
            double MuA = TeamRatingA.GetMu();
            double SigmaSquaredA = TeamRatingA.GetSigmaSquared();
            double MuB = TeamRatingB.GetMu();
            double SigmaSquaredB = TeamRatingB.GetSigmaSquared();
            double LocalDenom = gcem::sqrt(
                N * gcem::pow(Openskill::DefaultBeta, 2.0) + gcem::pow(SigmaSquaredA, 2.0) +
                gcem::pow(SigmaSquaredB, 2.0));
            PairwiseProbabilitiesSum += Openskill::Statistics::PhiMajor((DrawMargin - MuA + MuB) / LocalDenom) -
                                        Openskill::Statistics::PhiMajor((MuA - MuB - DrawMargin) / LocalDenom);
        }
    }

    double Denom = 1;
    if (N > 2)
    {
        Denom = N * (N - 1);
    }

    return gcem::abs(PairwiseProbabilitiesSum) / Denom;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()