// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakerSubmitMatchResults.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Matchmaker.h"
#include "OnlineSubsystemUtils.h"
#if WITH_EDITOR
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#endif

#include "RedpointMatchmaking/MatchmakingEngine.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

UMatchmakerSubmitMatchResults *UMatchmakerSubmitMatchResults::SubmitMatchResults(
    UMatchmakerSubsystem *Subsystem,
    FUniqueNetIdRepl LocalUserId,
    TArray<float> TeamScores)
{
    UMatchmakerSubmitMatchResults *Node = NewObject<UMatchmakerSubmitMatchResults>();
    Node->Subsystem = Subsystem;
    Node->LocalUserId = MoveTemp(LocalUserId);
    Node->TeamScores = MoveTemp(TeamScores);
    return Node;
}

void UMatchmakerSubmitMatchResults::Activate()
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!IsValid(this->Subsystem))
    {
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Matchmaker subsystem not valid."));
        this->OnComplete.Broadcast(false);
        return;
    }

    UWorld *World = this->Subsystem->GetWorld();
    if (!IsValid(World))
    {
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Current world not valid."));
        this->OnComplete.Broadcast(false);
        return;
    }

    if (this->Subsystem->CachedSkillStatPrefix.IsEmpty())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Attempted to call SubmitMatchResults, but you're either a client in the match, or the last completed "
                 "matchmaking result did not have skill-based matchmaking enabled."));
        this->OnComplete.Broadcast(false);
        return;
    }

    IOnlineStatsPtr Stats = Online::GetSubsystem(World)->GetStatsInterface();
    if (!Stats.IsValid())
    {
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Stats interface not valid for submit."));
        this->OnComplete.Broadcast(false);
        return;
    }

    TArray<double> TeamScoresDbl;
    for (const float &TeamScore : this->TeamScores)
    {
        TeamScoresDbl.Add((double)TeamScore);
    }

    if (!this->LocalUserId.IsValid())
    {
        if (World->GetNetMode() == NM_DedicatedServer)
        {
            IOnlineIdentityPtr Identity = Online::GetIdentityInterface(World);
            if (!Identity.IsValid())
            {
                UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Identity interface not valid."));
                this->OnComplete.Broadcast(false);
                return;
            }
            this->LocalUserId = Identity->GetUniquePlayerId(0);
        }
        else
        {
            APlayerController *PC = World->GetFirstPlayerController();
            if (IsValid(PC))
            {
                APlayerState *PS = PC->GetPlayerState<APlayerState>();
                if (IsValid(PS))
                {
                    this->LocalUserId = PS->GetUniqueId();
                }
            }
            if (!this->LocalUserId.IsValid())
            {
                IOnlineIdentityPtr Identity = Online::GetIdentityInterface(World);
                if (!Identity.IsValid())
                {
                    UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Identity interface not valid."));
                    this->OnComplete.Broadcast(false);
                    return;
                }
                this->LocalUserId = Identity->GetUniquePlayerId(0);
            }
        }
    }

    if (!this->LocalUserId.IsValid())
    {
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Local user ID is not valid."));
        this->OnComplete.Broadcast(false);
        return;
    }

    if (TeamScoresDbl.Num() != this->Subsystem->CachedResponse.Teams.Num())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("You only provided scores for %d teams, but %d teams were involved in matchmaking. You must provide a "
                 "score for every team."),
            TeamScoresDbl.Num(),
            this->Subsystem->CachedResponse.Teams.Num());
        this->OnComplete.Broadcast(false);
        return;
    }

    IMatchmakingEngine::Get()->SubmitMatchResults(
        this->LocalUserId.GetUniqueNetId().ToSharedRef(),
        Stats.ToSharedRef(),
        this->Subsystem->CachedSkillStatPrefix,
        this->Subsystem->CachedResponse,
        TeamScoresDbl,
        FMatchmakingEngineSubmitComplete::CreateUObject(this, &UMatchmakerSubmitMatchResults::OnCallback));
#else
    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
    this->OnComplete.Broadcast(false);
#endif
}

#if !REDPOINT_EOS_IS_FREE_EDITION

void UMatchmakerSubmitMatchResults::OnCallback(const FOnlineError &Error)
{
    if (!Error.bSucceeded)
    {
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Error while submitting match results: %s"), *Error.ToLogString());
    }

    this->OnComplete.Broadcast(Error.bSucceeded);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()