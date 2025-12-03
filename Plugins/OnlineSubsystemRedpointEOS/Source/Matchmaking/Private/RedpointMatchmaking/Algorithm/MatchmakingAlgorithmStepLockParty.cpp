// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepLockParty.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCreateMatchmakingFollowRequestLobby.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingPerformSearch.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateLockParty> FMatchmakingAlgorithmStepLockParty::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateLockParty>();
}

void FMatchmakingAlgorithmStepLockParty::OnPartyLocked(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const EUpdateConfigCompletionResult Result,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_LOCK_PARTY))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        switch (Result)
        {
        case EUpdateConfigCompletionResult::UnknownClientFailure:
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking: UnknownClientFailure"));
            return;
        case EUpdateConfigCompletionResult::UnknownParty:
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking: UnknownParty"));
            return;
        case EUpdateConfigCompletionResult::LocalMemberNotMember:
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking: LocalMemberNotMember"));
            return;
        case EUpdateConfigCompletionResult::LocalMemberNotLeader:
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking: LocalMemberNotLeader"));
            return;
        case EUpdateConfigCompletionResult::RemoteMemberNotMember:
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking: RemoteMemberNotMember"));
            return;
        case EUpdateConfigCompletionResult::MessagingFailure:
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking: MessagingFailure"));
            return;
        case EUpdateConfigCompletionResult::NoResponse:
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking: NoResponse"));
            return;
        case EUpdateConfigCompletionResult::UnknownInternalFailure:
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking: UnknownInternalFailure"));
            return;
        case EUpdateConfigCompletionResult::Succeeded:
        default:
            break;
        }

        if (Result != EUpdateConfigCompletionResult::Succeeded)
        {
            this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking"));
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sFinished locking the party (%s)"),
            *this->GetLogPrefix(*Task),
            *Task->Request.PartyId->ToString());

        TArray<FOnlinePartyMemberConstRef> Members;
        if (!Pins->OnlineParty->GetPartyMembers(LocalUserId, PartyId, Members))
        {
            this->RestartableError(*Task, TEXT("Unable to get party members to store as pending players"));
            return;
        }

        if (!Task->Request.HostConfiguration->SkillStatPrefix.IsEmpty() && Pins->OnlineStats.IsValid())
        {
            // We have to load the stats for the party.
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sNeed to load party skill stats for skill-based matchmaking"),
                *this->GetLogPrefix(*Task));

            TUserIdSet UsersToLoadStatsFor;
            UsersToLoadStatsFor.Add(*Task->Request.UserId);
            for (const auto &PartyMember : Members)
            {
                UsersToLoadStatsFor.Add(*PartyMember->GetUserId());
            }

            TArray<FString> StatNames;
            StatNames.Add(FString::Printf(TEXT("%s_mu"), *Task->Request.HostConfiguration->SkillStatPrefix));
            StatNames.Add(FString::Printf(TEXT("%s_sigma"), *Task->Request.HostConfiguration->SkillStatPrefix));
            TArray<TSharedRef<const FUniqueNetId>> TargetUserIds;
            for (const auto &TargetUserId : UsersToLoadStatsFor)
            {
                TargetUserIds.Add(TargetUserId);
            }

            Pins->OnlineStats->QueryStats(
                Task->Request.UserId.ToSharedRef(),
                TargetUserIds,
                StatNames,
                FOnlineStatsQueryUsersStatsComplete::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepLockParty::OnPartyStatsCallback,
                    LocalUserId.AsShared(),
                    PartyId.AsShared(),
                    UsersToLoadStatsFor,
                    TWeakPtr<FMatchmakingEngineTask>(Task)));
        }
        else
        {
            // Put the current party members into the pending users
            // list.
            TUserIdMap<FMatchmakingTeamId> InitialPendingBatch;
            for (const auto &Member : Members)
            {
                // Initial pending users must all go onto team 0, same as the host user.
                InitialPendingBatch.Add(*Member->GetUserId(), 0);
            }
            if (!Task->TeamManager->TryAddPendingBatch(InitialPendingBatch, TUserIdMap<FOpenskillRating>()))
            {
                this->PermanentError(*Task, OnlineMatchmaking::Errors::PartyTooLargeForFirstTeam());
                return;
            }

            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_CREATE_MATCHMAKING_FOLLOW_REQUEST_LOBBY);
        }
    }
}

void FMatchmakingAlgorithmStepLockParty::OnPartyStatsCallback(
    const FOnlineError &Error,
    const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
    TSharedRef<const FUniqueNetId> LocalUserId,
    TSharedRef<const FOnlinePartyId> PartyId,
    TUserIdSet UsersToLoadStatsFor,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_LOCK_PARTY))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (!Error.bSucceeded)
        {
            if (Error.ErrorCode.EndsWith(".timed_out"))
            {
                this->PermanentError(*Task, OnlineMatchmaking::Errors::FailedToLoadEssentialSkillStats());
                return;
            }

            this->PermanentError(*Task, OnlineMatchmaking::Errors::FailedToLoadEssentialSkillStats());
            return;
        }

        TUserIdMap<FOpenskillRating> LoadedRatings =
            FMatchmakingAlgorithmStepSearchingPerformSearch::DeserializeOpenskillStats(
                Task->Request.HostConfiguration->SkillStatPrefix,
                UsersStatsResult,
                UsersToLoadStatsFor);

        // Set the host's Openskill rating.
        Task->TeamManager->SetHostOpenskillRating(LoadedRatings[*Task->Request.UserId]);

        // Put the current party members into the pending users
        // list.
        TArray<FOnlinePartyMemberConstRef> Members;
        if (!Pins->OnlineParty->GetPartyMembers(*LocalUserId, *PartyId, Members))
        {
            this->RestartableError(*Task, TEXT("Unable to get party members to store as pending players"));
            return;
        }
        TUserIdMap<FMatchmakingTeamId> InitialPendingBatch;
        for (const auto &Member : Members)
        {
            // Initial pending users must all go onto team 0, same as the host user.
            InitialPendingBatch.Add(*Member->GetUserId(), 0);
        }
        if (!Task->TeamManager->TryAddPendingBatch(InitialPendingBatch, LoadedRatings))
        {
            this->PermanentError(*Task, OnlineMatchmaking::Errors::PartyTooLargeForFirstTeam());
            return;
        }

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_CREATE_MATCHMAKING_FOLLOW_REQUEST_LOBBY);
    }
}

void FMatchmakingAlgorithmStepLockParty::OnSoloStatsCallback(
    const FOnlineError &Error,
    const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
    TUserIdSet UsersToLoadStatsFor,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_LOCK_PARTY))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (!Error.bSucceeded)
        {
            if (Error.ErrorCode.EndsWith(".timed_out"))
            {
                this->PermanentError(*Task, OnlineMatchmaking::Errors::FailedToLoadEssentialSkillStats());
                return;
            }

            this->PermanentError(*Task, OnlineMatchmaking::Errors::FailedToLoadEssentialSkillStats());
            return;
        }

        TUserIdMap<FOpenskillRating> LoadedRatings =
            FMatchmakingAlgorithmStepSearchingPerformSearch::DeserializeOpenskillStats(
                Task->Request.HostConfiguration->SkillStatPrefix,
                UsersStatsResult,
                UsersToLoadStatsFor);

        Task->TeamManager->SetHostOpenskillRating(LoadedRatings[*Task->Request.UserId]);

        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_CREATE_MATCHMAKING_FOLLOW_REQUEST_LOBBY);
    }
}

void FMatchmakingAlgorithmStepLockParty::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        if (Task->Request.PartyId != nullptr)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sLocking the party (%s) so that other people can't join while matchmaking"),
                *this->GetLogPrefix(*Task),
                *Task->Request.PartyId->ToString());

            FOnlinePartyConstPtr PartyInfo = Pins->OnlineParty->GetParty(*Task->Request.UserId, *Task->Request.PartyId);
            if (PartyInfo != nullptr)
            {
                TSharedRef<FPartyConfiguration> Config =
                    MakeShared<FPartyConfiguration>(*PartyInfo->GetConfiguration());
                if (Config->bIsAcceptingMembers)
                {
                    // Technically we can run through LockParty multiple times due to abandonment, but since we
                    // don't reset to the Task state, it will still result in the party being unlocked on completion.
                    Task->PartyMustBeUnlocked = true;
                    Config->bIsAcceptingMembers = false;
                    if (!Pins->OnlineParty->UpdateParty(
                            *Task->Request.UserId,
                            *Task->Request.PartyId,
                            *Config,
                            false,
                            FOnUpdatePartyComplete::CreateSP(
                                this,
                                &FMatchmakingAlgorithmStepLockParty::OnPartyLocked,
                                TWeakPtr<FMatchmakingEngineTask>(Task))))
                    {
                        this->RestartableError(*Task, TEXT("Unable to lock party for matchmaking"));
                        return;
                    }
                }
                else
                {
                    // Party is already locked.
                    this->OnPartyLocked(
                        *Task->Request.UserId,
                        *Task->Request.PartyId,
                        EUpdateConfigCompletionResult::Succeeded,
                        TWeakPtr<FMatchmakingEngineTask>(Task));
                }
            }
        }
        else
        {
            if (!Task->Request.HostConfiguration->SkillStatPrefix.IsEmpty() && Pins->OnlineStats.IsValid())
            {
                // We have to load the stats for the host (no party).
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sNeed to load host skill stat for skill-based matchmaking"),
                    *this->GetLogPrefix(*Task));

                TUserIdSet UsersToLoadStatsFor;
                UsersToLoadStatsFor.Add(*Task->Request.UserId);

                TArray<FString> StatNames;
                StatNames.Add(FString::Printf(TEXT("%s_mu"), *Task->Request.HostConfiguration->SkillStatPrefix));
                StatNames.Add(FString::Printf(TEXT("%s_sigma"), *Task->Request.HostConfiguration->SkillStatPrefix));
                TArray<TSharedRef<const FUniqueNetId>> TargetUserIds;
                for (const auto &TargetUserId : UsersToLoadStatsFor)
                {
                    TargetUserIds.Add(TargetUserId);
                }

                Pins->OnlineStats->QueryStats(
                    Task->Request.UserId.ToSharedRef(),
                    TargetUserIds,
                    StatNames,
                    FOnlineStatsQueryUsersStatsComplete::CreateSP(
                        this,
                        &FMatchmakingAlgorithmStepLockParty::OnSoloStatsCallback,
                        UsersToLoadStatsFor,
                        TWeakPtr<FMatchmakingEngineTask>(Task)));
            }
            else
            {
                EMIT_TASK_COUNTER_TRACE(Task);
                this->MoveToStep(*Task, MMSTEP_CREATE_MATCHMAKING_FOLLOW_REQUEST_LOBBY);
            }
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()