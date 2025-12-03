// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "./OpenskillRating.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

struct FOpenskillTeam
{
private:
    TArray<FOpenskillRating> Players;

public:
    FOpenskillTeam() = delete;
    FOpenskillTeam(const TArray<FOpenskillRating> &InPlayers)
        : Players(InPlayers)
    {
    }
    FOpenskillTeam(const FOpenskillTeam &In) = default;
    FOpenskillTeam(FOpenskillTeam &&In) = default;
    ~FOpenskillTeam() = default;

    const TArray<FOpenskillRating> &GetPlayerRatings() const
    {
        return this->Players;
    }
};

class MATCHMAKING_API FOpenskill
{
public:
    /**
     * Given a set of ranked teams, compute the new ratings for each player.
     *
     * Teams should be provided in the order "winners to losers". That is, if you have two teams competing with each
     * other, the first team would be the winner and the second team would be the loser. For games with more than 2
     * teams, you need to rank teams from "highest scoring" to "lowest scoring" and pass them in that order.
     */
    static TArray<FOpenskillTeam> Rate(
        const TArray<FOpenskillTeam> &OrderedTeams,
        const TArray<double> &TeamRankings = TArray<double>());

    /**
     * For a given theoretical arrangement of players, compute the chance each team will win the match.
     *
     * For a balanced match, you'll want the variance between each team's chance to be as low as possible.
     */
    static TArray<double> PredictWinners(const TArray<FOpenskillTeam> &Teams);

    /**
     * For a given theoretical arrangement of players, compute the chance that the match will be a draw.
     */
    static double PredictDraw(const TArray<FOpenskillTeam> &Teams);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()