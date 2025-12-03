// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "./Openskill/OpenskillRating.h"
#include "MatchmakingEngineTask.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

struct FMatchmakingSuitableMatchResult
{
private:
    FMatchmakingSuitableMatchResult() = default;

public:
    FMatchmakingSuitableMatchResult(
        const TSharedRef<const FOnlineLobbyId> &InCandidateLobbyId,
        const FString &InStateHash)
        : CandidateLobbyId(TSharedPtr<const FOnlineLobbyId>(InCandidateLobbyId))
        , Suitable(false)
        , TeamReassignments()
        , MemberSkillRating()
        , SuitabilityScore()
        , UnsuitabilityReason()
        , StateHash(InStateHash)
    {
    }

    static FMatchmakingSuitableMatchResult Invalid()
    {
        return FMatchmakingSuitableMatchResult();
    }

    TSharedPtr<const FOnlineLobbyId> CandidateLobbyId;
    bool Suitable;
    TUserIdMap<FMatchmakingTeamId> TeamReassignments;
    TUserIdMap<FOpenskillRating> MemberSkillRating;
    int SuitabilityScore;
    FString UnsuitabilityReason;
    FString StateHash;
};

struct FMatchmakingCandidateLobby
{
public:
    TSharedRef<const FOnlineLobbyId> LobbyId;
    FString StateHash;
    TArray<TSharedRef<const FUniqueNetId>> MemberIds;
    TUserIdMap<FMatchmakingTeamId> MemberTeams;
    TUserIdMap<FOpenskillRating> MemberSkillRating;
    bool bSkillEnabled;

    FMatchmakingCandidateLobby(const FOnlineLobbyId &InLobbyId, const FString &InStateHash)
        : LobbyId(InLobbyId.AsShared())
        , StateHash(InStateHash)
        , MemberIds()
        , MemberTeams()
        , MemberSkillRating()
        , bSkillEnabled(false)
    {
    }
    UE_NONCOPYABLE(FMatchmakingCandidateLobby);
    ~FMatchmakingCandidateLobby() = default;
};

class IMatchmakingCandidateEvaluator
{
public:
    virtual ~IMatchmakingCandidateEvaluator() = default;

    virtual FMatchmakingSuitableMatchResult EvaluateCandidate(
        FMatchmakingEngineTask &Task,
        const FMatchmakingCandidateLobby &Candidate,
        double SecondsRemaining,
        double IterationMaximumSecondsRemaining) = 0;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()