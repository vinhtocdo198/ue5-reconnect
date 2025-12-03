// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerTeamManagement.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "../Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestReject.h"
#include "../Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestVerify.h"
#include "../MatchmakingFollowRequest.h"
#include "../MatchmakingTeamManager.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFollowHost.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_TEAM_MANAGEMENT FName("EventHandlerTeamManagement")

#define MMHANDLER_TEAM_MANAGEMENT_CONNECT FName("EventHandlerTeamManagementConnect")
#define MMHANDLER_TEAM_MANAGEMENT_DISCONNECT FName("EventHandlerTeamManagementDisconnect")

void FMatchmakingEventHandlerTeamManagement::OnMemberConnect_LobbyUpdated(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedRef<const FUniqueNetId> MemberId,
    FMatchmakingTeamId Team)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (*Task->Request.UserId != UserId)
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                auto Txn =
                    Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
                Txn->SetMetadata.Add(
                    FString::Printf(TEXT("%s.%s"), *MemberId->ToString(), MatchmakingMemberMetadataTeam),
                    FVariantData((int64)Team));
                Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());
                if (!Pins->OnlineLobby->UpdateLobby(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingEventHandlerTeamManagement::OnMemberConnect_LobbyUpdated,
                            MoveTemp(Engine),
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            MemberId,
                            Team)))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Error,
                        TEXT("%sUnable to update lobby on member connect (retry for timed_out) with assignment of %s "
                             "to team %d: %s"),
                        *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
                        *MemberId->ToString(),
                        Team,
                        TEXT("UpdateLobby call did not start!"));
                }
                return;
            }
        }

        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sUnable to update lobby on member connect with assignment of %s to team %d: %s"),
            *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
            *MemberId->ToString(),
            Team,
            *Error.ToLogString());
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sUser %s was assigned to team %d."),
            *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
            *MemberId->ToString(),
            Team);
    }

    if (Task->TeamManager->HasPendingBatch() && Task->TeamManager->IsPendingBatchReady())
    {
        Task->TeamManager->FinalizePendingBatch();

        auto Step =
            Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);
        if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ALL_PLAYERS_HAVE_CONNECTED))
        {
            verify(Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_ALL_PLAYERS_HAVE_CONNECTED));
        }
    }
}

void FMatchmakingEventHandlerTeamManagement::OnMemberConnect(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (Task->TeamManager.IsValid() && Task->MatchmakingLobbyId.IsValid() && *Task->MatchmakingLobbyId == LobbyId &&
        *Task->Request.UserId == UserId)
    {
        if (auto Pins = Task->PinAndContinue())
        {
            if (!Task->TeamManager->TryMarkAsConnected(MemberId))
            {
                // This user wasn't expected to connect. Kick them.
                if (!Pins->OnlineLobby->KickMember(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        MemberId,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingEventHandlerTeamManagement::OnMemberConnect_MemberKicked,
                            MoveTemp(Engine),
                            TaskWk,
                            MemberId.AsShared())))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Error,
                        TEXT("%sUnable to kick unexpected member %s: KickMember operation did not start"),
                        *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
                        *MemberId.ToString());
                    return;
                }

                return;
            }

            FMatchmakingTeamId Team = Task->TeamManager->GetAssignedTeam(MemberId);
            checkf(
                Team != EMatchmakingTeamId_None,
                TEXT("TryMarkAsConnected must not return true if there is no assigned team! The member ID was %s. The "
                     "value returned from GetAssignedTeam was %d."),
                *MemberId.ToString(),
                Team);

            auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
            Txn->SetMetadata.Add(
                FString::Printf(TEXT("%s.%s"), *MemberId.ToString(), MatchmakingMemberMetadataTeam),
                FVariantData((int64)Team));
            Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());

            if (!Pins->OnlineLobby->UpdateLobby(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    *Txn,
                    FOnLobbyOperationComplete::CreateSP(
                        this,
                        &FMatchmakingEventHandlerTeamManagement::OnMemberConnect_LobbyUpdated,
                        MoveTemp(Engine),
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        MemberId.AsShared(),
                        Team)))
            {
                UE_LOG(
                    LogMatchmaking,
                    Error,
                    TEXT("%sUnable to update lobby on member connect with assignment of %s to team %d: %s"),
                    *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
                    *MemberId.ToString(),
                    Team,
                    TEXT("UpdateLobby call did not start!"));
                return;
            }
        }
    }
}

void FMatchmakingEventHandlerTeamManagement::OnMemberDisconnect_LobbyUpdated(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedRef<const FUniqueNetId> MemberId,
    FMatchmakingTeamId Team)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    if (*Task->Request.UserId != UserId)
    {
        return;
    }

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                // Retry on timeout.
                auto Txn =
                    Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
                Txn->DeleteMetadata.Add(
                    FString::Printf(TEXT("%s.%s"), *MemberId->ToString(), MatchmakingMemberMetadataTeam));
                Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());
                if (!Pins->OnlineLobby->UpdateLobby(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *Txn,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingEventHandlerTeamManagement::OnMemberDisconnect_LobbyUpdated,
                            MoveTemp(Engine),
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            MemberId,
                            Team)))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Error,
                        TEXT("%sUnable to update lobby data to indicate that %s on team %d is no longer connected on "
                             "retry timed_out..."),
                        *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
                        *MemberId->ToString(),
                        Team);
                }
                return;
            }
        }

        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sUnable to update lobby data to indicate that %s on team %d is no longer connected..."),
            *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
            *MemberId->ToString(),
            Team,
            *Error.ToLogString());
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sUser %s was unassigned from team %d."),
            *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
            *MemberId->ToString(),
            Team);
    }
}

void FMatchmakingEventHandlerTeamManagement::OnMemberDisconnect(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    bool bWasKicked,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (Task->TeamManager.IsValid() && Task->MatchmakingLobbyId.IsValid() && *Task->MatchmakingLobbyId == LobbyId &&
        *Task->Request.UserId == UserId)
    {
        if (*Task->Request.UserId == MemberId)
        {
            // The current user is the one that disconnected, so we won't be able to perform update operations
            // on the lobby (as we are no longer a member).
            return;
        }

        if (auto Pins = Task->PinAndContinue())
        {
            // Check if the matchmaking result was already finalized. If so, don't touch our team management state.
            if (Task->StoredResponse.IsValid())
            {
                UE_LOG(
                    LogMatchmaking,
                    Error,
                    TEXT("%sIgnoring member change in team management, because we already have a finalized response."),
                    *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT));
                EMIT_TASK_COUNTER_TRACE(Task);
                return;
            }

            FMatchmakingTeamId PreviousTeam = Task->TeamManager->GetAssignedTeam(MemberId);

            Task->TeamManager->MarkAsDisconnected(MemberId);

            auto Txn = Pins->OnlineLobby->MakeUpdateLobbyTransaction(*Task->Request.UserId, *Task->MatchmakingLobbyId);
            Txn->DeleteMetadata.Add(
                FString::Printf(TEXT("%s.%s"), *MemberId.ToString(), MatchmakingMemberMetadataTeam));
            Txn->SetMetadata.Add(MatchmakingMetadataLobbyHeartbeatTimestamp, FDateTime::UtcNow().ToUnixTimestamp());

            if (!Pins->OnlineLobby->UpdateLobby(
                    *Task->Request.UserId,
                    *Task->MatchmakingLobbyId,
                    *Txn,
                    FOnLobbyOperationComplete::CreateSP(
                        this,
                        &FMatchmakingEventHandlerTeamManagement::OnMemberDisconnect_LobbyUpdated,
                        MoveTemp(Engine),
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        MemberId.AsShared(),
                        PreviousTeam)))
            {
                UE_LOG(
                    LogMatchmaking,
                    Error,
                    TEXT("%sUnable to update lobby data to indicate that %s on team %d is no longer connected..."),
                    *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
                    *MemberId.ToString(),
                    PreviousTeam);
                return;
            }
        }
    }
}

void FMatchmakingEventHandlerTeamManagement::RegisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_TEAM_MANAGEMENT_CONNECT,
            Pins->OnlineLobby->AddOnMemberConnectDelegate_Handle(FOnMemberConnectDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerTeamManagement::OnMemberConnect,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        Task.EventHandlers.Add(
            MMHANDLER_TEAM_MANAGEMENT_DISCONNECT,
            Pins->OnlineLobby->AddOnMemberDisconnectDelegate_Handle(FOnMemberDisconnectDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerTeamManagement::OnMemberDisconnect,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerTeamManagement::UnregisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_TEAM_MANAGEMENT_CONNECT))
        {
            Pins->OnlineLobby->ClearOnMemberConnectDelegate_Handle(
                Task.EventHandlers[MMHANDLER_TEAM_MANAGEMENT_CONNECT]);
            Task.EventHandlers.Remove(MMHANDLER_TEAM_MANAGEMENT_CONNECT);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        }
        if (Task.EventHandlers.Contains(MMHANDLER_TEAM_MANAGEMENT_DISCONNECT))
        {
            Pins->OnlineLobby->ClearOnMemberDisconnectDelegate_Handle(
                Task.EventHandlers[MMHANDLER_TEAM_MANAGEMENT_DISCONNECT]);
            Task.EventHandlers.Remove(MMHANDLER_TEAM_MANAGEMENT_DISCONNECT);
            EMIT_TASK_COUNTER_TRACE(Task.AsShared());
        }
    }
}

void FMatchmakingEventHandlerTeamManagement::OnMemberConnect_MemberKicked(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedRef<const FUniqueNetId> MemberId)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Error.bSucceeded)
    {
        if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
        {
            if (auto Pins = Task->PinAndContinue())
            {
                if (!Pins->OnlineLobby->KickMember(
                        *Task->Request.UserId,
                        *Task->MatchmakingLobbyId,
                        *MemberId,
                        FOnLobbyOperationComplete::CreateSP(
                            this,
                            &FMatchmakingEventHandlerTeamManagement::OnMemberConnect_MemberKicked,
                            MoveTemp(Engine),
                            TaskWk,
                            MemberId)))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Error,
                        TEXT("%sUnable to kick unexpected member %s on retry timed_out: KickMember operation did not "
                             "start"),
                        *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
                        *MemberId->ToString());
                }
                return;
            }
        }

        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%sUnable to kick unexpected member %s: %s"),
            *this->GetLogPrefix(*Task, MMHANDLER_TEAM_MANAGEMENT),
            *MemberId->ToString(),
            *Error.ToLogString());
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()