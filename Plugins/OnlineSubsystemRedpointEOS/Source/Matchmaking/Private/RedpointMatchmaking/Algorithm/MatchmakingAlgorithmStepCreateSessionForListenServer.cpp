// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCreateSessionForListenServer.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlineSessionInterface.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCreateSessionForListenServer>
FMatchmakingAlgorithmStepCreateSessionForListenServer::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateCreateSessionForListenServer>();
}

void FMatchmakingAlgorithmStepCreateSessionForListenServer::OnSessionCreated(
    FName SessionName,
    bool bWasSuccessful,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    TSharedPtr<FMatchmakingEngineTask> Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_CREATE_SESSION_FOR_LISTEN_SERVER))
    {
        return;
    }

    if (!SessionName.IsEqual(NAME_GameSession))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        auto State = this->GetState(*Task);
        Pins->OnlineSession->ClearOnCreateSessionCompleteDelegate_Handle(State->SessionCreateHandle);

        if (!bWasSuccessful)
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sFailed to create game session for listen server"),
                *this->GetLogPrefix(*Task));
            this->PermanentError(*Task, OnlineMatchmaking::Errors::UnableToCreateGameSession());
        }
        else
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sCreated game session for listen server"),
                *this->GetLogPrefix(*Task));
            this->MoveToStep(*Task, MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING_FOR_LISTEN_SERVER);
        }
    }
}

void FMatchmakingAlgorithmStepCreateSessionForListenServer::CreateSession(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        int SessionCapacity = 0;
        for (const auto &Team : Task->StoredResponse->Teams)
        {
            SessionCapacity += Team.Slots.Num();
        }

        FOnlineSessionSettings Settings = {};
        Settings.NumPublicConnections = SessionCapacity;
        Settings.NumPrivateConnections = 0;
        Settings.bShouldAdvertise = true;
        Settings.bAllowJoinInProgress = false;
        Settings.bIsLANMatch = false;
        Settings.bIsDedicated = false;
        Settings.bUsesStats = false;
        Settings.bAllowInvites = false;
        Settings.bAllowJoinViaPresence = false;
        Settings.bAllowJoinViaPresenceFriendsOnly = false;
        Settings.bAntiCheatProtected = false;
        Settings.bUseLobbiesIfAvailable = true;
        Settings.bUseLobbiesVoiceChatIfAvailable = false;
        Settings.BuildUniqueId = 0;
        Settings.Settings.Reset();
        Settings.Settings.Add(
            FName("HostUserId"),
            FOnlineSessionSetting(Task->Request.UserId->ToString(), EOnlineDataAdvertisementType::ViaOnlineService));
        Settings.Settings.Add(
            FName("HostTaskId"),
            FOnlineSessionSetting(Task->TaskId, EOnlineDataAdvertisementType::ViaOnlineService));

        UE_LOG(LogMatchmaking, Verbose, TEXT("%sCreating game session for listen server"), *this->GetLogPrefix(*Task));

        auto State = this->GetState(*Task);
        State->SessionCreateHandle =
            Pins->OnlineSession->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateSP(
                this,
                &FMatchmakingAlgorithmStepCreateSessionForListenServer::OnSessionCreated,
                TWeakPtr<FMatchmakingEngineTask>(Task)));
        if (!Pins->OnlineSession->CreateSession(*Task->Request.UserId, NAME_GameSession, Settings))
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%sFailed to create game session for listen server"),
                *this->GetLogPrefix(*Task));

            Pins->OnlineSession->ClearOnCreateSessionCompleteDelegate_Handle(State->SessionCreateHandle);
            this->PermanentError(*Task, OnlineMatchmaking::Errors::UnableToCreateGameSession());
        }
    }
}

void FMatchmakingAlgorithmStepCreateSessionForListenServer::OnSessionDestroyed(
    FName SessionName,
    bool bWasSuccessful,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    if (!bWasSuccessful)
    {
        UE_LOG(
            LogMatchmaking,
            Warning,
            TEXT("%sFailed to destroy existing session; attempting to create session anyway."),
            *this->GetLogPrefix(*Task));
    }

    this->CreateSession(Task.ToSharedRef());
}

void FMatchmakingAlgorithmStepCreateSessionForListenServer::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        auto *ExistingSession = Pins->OnlineSession->GetNamedSession(NAME_GameSession);
        if (ExistingSession == nullptr)
        {
            this->CreateSession(Task);
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sDestroying existing game session for listen server"),
            *this->GetLogPrefix(*Task));

        auto State = this->GetState(*Task);
        if (!Pins->OnlineSession->DestroySession(
                NAME_GameSession,
                FOnDestroySessionCompleteDelegate::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepCreateSessionForListenServer::OnSessionDestroyed,
                    TWeakPtr<FMatchmakingEngineTask>(Task))))
        {
            UE_LOG(
                LogMatchmaking,
                Warning,
                TEXT("%sFailed to destroy existing session; attempting to create session anyway."),
                *this->GetLogPrefix(*Task));
            this->CreateSession(Task);
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()