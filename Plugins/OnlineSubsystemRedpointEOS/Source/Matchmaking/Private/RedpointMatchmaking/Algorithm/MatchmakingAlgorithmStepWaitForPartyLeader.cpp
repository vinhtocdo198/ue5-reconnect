// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepWaitForPartyLeader.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFollowHost.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateWaitForPartyLeader> FMatchmakingAlgorithmStepWaitForPartyLeader::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateWaitForPartyLeader>();
}

void FMatchmakingAlgorithmStepWaitForPartyLeader::OnConnected(
    const FOnlineError &Error,
    const FUniqueNetId &UserId,
    const TSharedPtr<class FOnlineLobby> &Lobby,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedPtr<FOnlineLobbyId> TargetLobbyId)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_WAIT_FOR_PARTY_LEADER))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (Error.bSucceeded)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sClient successfully connected to lobby: %s"),
                *this->GetLogPrefix(*Task),
                *TargetLobbyId->ToString());

            // Update the task state so that the ClientFollowing event handler can work.
            Task->MatchmakingLobbyId = TargetLobbyId;

            EMIT_TASK_COUNTER_TRACE(Task);
            this->MoveToStep(*Task, MMSTEP_FOLLOW_HOST);
        }
        else
        {
            if (Error.ErrorCode.EndsWith(".timed_out") || Error.ErrorCode.EndsWith(".already_pending"))
            {
                // Retry joining.
                UE_LOG(
                    LogMatchmaking,
                    Verbose,
                    TEXT("%sUser is joining party leader's lobby: %s"),
                    *this->GetLogPrefix(*Task),
                    *TargetLobbyId->ToString());

                if (!Pins->OnlineLobby->ConnectLobby(
                        *Task->Request.UserId,
                        *TargetLobbyId,
                        FOnLobbyCreateOrConnectComplete::CreateSP(
                            this,
                            &FMatchmakingAlgorithmStepWaitForPartyLeader::OnConnected,
                            TWeakPtr<FMatchmakingEngineTask>(Task),
                            TargetLobbyId)))
                {
                    UE_LOG(
                        LogMatchmaking,
                        Error,
                        TEXT("%sUnable to start ConnectLobby task to join party leader's lobby on retry: %s"),
                        *this->GetLogPrefix(*Task),
                        *TargetLobbyId->ToString());

                    this->PermanentError(*Task, OnlineMatchmaking::Errors::ClientUnableToJoinHostLobby());
                }
            }
            else
            {
                UE_LOG(
                    LogMatchmaking,
                    Error,
                    TEXT("%sUnable to start ConnectLobby task to join party leader's lobby '%s': %s"),
                    *this->GetLogPrefix(*Task),
                    *TargetLobbyId->ToString(),
                    *Error.ToLogString());

                this->PermanentError(*Task, OnlineMatchmaking::Errors::ClientUnableToJoinHostLobby());
            }
        }
    }
}

void FMatchmakingAlgorithmStepWaitForPartyLeader::OnPartyDataUpdated(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
    const FOnlinePartyData &PartyData,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_WAIT_FOR_PARTY_LEADER))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        FVariantData OutLobbyId;
        if (LocalUserId == *Task->Request.UserId && PartyId == *Task->Request.PartyId &&
            PartyData.GetAttribute(PartyMetadataMatchmakingLobbyId, OutLobbyId) &&
            OutLobbyId.GetType() == EOnlineKeyValuePairDataType::String)
        {
            FString OutLobbyIdStr;
            OutLobbyId.GetValue(OutLobbyIdStr);

            // The party leader has told us we need to join a matchmaking lobby.
            TSharedPtr<FOnlineLobbyId> LobbyId = Pins->OnlineLobby->ParseSerializedLobbyId(OutLobbyIdStr);
            if (!LobbyId.IsValid())
            {
                UE_LOG(
                    LogMatchmaking,
                    Error,
                    TEXT("%sLobby we were told to join by party leader isn't parsable: %s"),
                    *this->GetLogPrefix(*Task),
                    *OutLobbyIdStr);
            }

            TSharedPtr<FMatchmakingAlgorithmStepStateWaitForPartyLeader> State = GetState(*Task);
            Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(State->OnPartyDataReceivedHandle);
            Pins->OnlineParty->ClearOnPartyExitedDelegate_Handle(State->OnPartyExitedHandle);

            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sUser is joining party leader's lobby: %s"),
                *this->GetLogPrefix(*Task),
                *OutLobbyIdStr);

            if (!Pins->OnlineLobby->ConnectLobby(
                    *Task->Request.UserId,
                    *LobbyId,
                    FOnLobbyCreateOrConnectComplete::CreateSP(
                        this,
                        &FMatchmakingAlgorithmStepWaitForPartyLeader::OnConnected,
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        LobbyId)))
            {
                UE_LOG(
                    LogMatchmaking,
                    Error,
                    TEXT("%sUnable to start ConnectLobby task to join party leader's lobby: %s"),
                    *this->GetLogPrefix(*Task),
                    *OutLobbyIdStr);

                this->PermanentError(*Task, OnlineMatchmaking::Errors::ClientUnableToJoinHostLobby());
            }
        }
    }
}

void FMatchmakingAlgorithmStepWaitForPartyLeader::OnPartyExited(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_WAIT_FOR_PARTY_LEADER))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (LocalUserId == *Task->Request.UserId && PartyId == *Task->Request.PartyId)
        {
            TSharedPtr<FMatchmakingAlgorithmStepStateWaitForPartyLeader> State = GetState(*Task);
            Pins->OnlineParty->ClearOnPartyDataReceivedDelegate_Handle(State->OnPartyDataReceivedHandle);
            Pins->OnlineParty->ClearOnPartyExitedDelegate_Handle(State->OnPartyExitedHandle);

            this->PermanentError(*Task, OnlineMatchmaking::Errors::ClientLeftParty());
        }
    }
}

void FMatchmakingAlgorithmStepWaitForPartyLeader::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    check(Task->Request.PartyId.IsValid());

    if (auto Pins = Task->PinAndContinue())
    {
        if (!Pins->OnlineParty->GetParty(*Task->Request.UserId, *Task->Request.PartyId).IsValid())
        {
            this->PermanentError(*Task, OnlineMatchmaking::Errors::ClientNotMemberOfParty());
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sWaiting for the party leader to tell us to join a lobby..."),
            *this->GetLogPrefix(*Task));

        TSharedPtr<FMatchmakingAlgorithmStepStateWaitForPartyLeader> State = GetState(*Task);
        State->OnPartyDataReceivedHandle =
            Pins->OnlineParty->AddOnPartyDataReceivedDelegate_Handle(FOnPartyDataReceivedDelegate::CreateSP(
                this,
                &FMatchmakingAlgorithmStepWaitForPartyLeader::OnPartyDataUpdated,
                TWeakPtr<FMatchmakingEngineTask>(Task)));
        State->OnPartyExitedHandle =
            Pins->OnlineParty->AddOnPartyExitedDelegate_Handle(FOnPartyExitedDelegate::CreateSP(
                this,
                &FMatchmakingAlgorithmStepWaitForPartyLeader::OnPartyExited,
                TWeakPtr<FMatchmakingEngineTask>(Task)));

        // Get the party data and simulate OnPartyDataUpdated call. This allows us to handle situations
        // where the host starts matchmaking before clients.
        FOnlinePartyDataConstPtr PartyData =
            Pins->OnlineParty->GetPartyData(*Task->Request.UserId, *Task->Request.PartyId, DefaultPartyDataNamespace);
        if (PartyData.IsValid())
        {
            FName Namespace = NAME_None;
            this->OnPartyDataUpdated(*Task->Request.UserId, *Task->Request.PartyId, Namespace, *PartyData, Task);
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()