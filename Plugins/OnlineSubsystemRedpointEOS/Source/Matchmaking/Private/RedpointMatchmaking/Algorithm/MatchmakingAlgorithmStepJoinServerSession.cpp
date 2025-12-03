// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepJoinServerSession.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "./MatchmakingAlgorithmStepConnectToGameServer.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

void FMatchmakingAlgorithmStepJoinServerSession::OnSessionFindComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FOnlineSessionSearchResult &SearchResult,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    FString SessionId,
    int32 AttemptNumber)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    if (auto Pins = Task->PinAndContinue())
    {
        if (!Pins->OnlineSession.IsValid())
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to finalise join by session ID: Session interface is not valid"));
            return;
        }

        if (bWasSuccessful)
        {
            // We joined successfully. Get the connection URL from the subsystem and then connect to the game server.
            FString ConnectionUrl;
            if (!Pins->OnlineSession->GetResolvedConnectString(SearchResult, NAME_GamePort, ConnectionUrl))
            {
                this->RestartableError(
                    *Task,
                    TEXT("Unable to finalise join by session ID: Call to GetResolvedConnectString failed for "
                         "NAME_GamePort"));
                return;
            }

            this->MoveToStep(*Task, MMSTEP_CONNECT_TO_GAME_SERVER, ConnectionUrl);
            return;
        }
        else if (AttemptNumber < 10)
        {
            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sTemporarily unable to find session to connect to, retrying with attempt #%d"),
                *this->GetLogPrefix(*Task),
                AttemptNumber + 1);

            if (!Pins->OnlineSession->FindSessionById(
                    *Task->Request.UserId,
                    *Pins->OnlineSession->CreateSessionIdFromString(SessionId),
                    *Task->Request.UserId,
                    FOnSingleSessionResultCompleteDelegate::CreateSP(
                        this,
                        &FMatchmakingAlgorithmStepJoinServerSession::OnSessionFindComplete,
                        TWeakPtr<FMatchmakingEngineTask>(Task),
                        SessionId,
                        AttemptNumber + 1)))
            {
                this->RestartableError(
                    *Task,
                    TEXT("Unable to find game by session ID: Call to FindSessionById failed, try setting "
                         "bConnectViaSessionID to false"));
                return;
            }
        }
        else
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to find game session ID: Tried 10 times and could not find the session by it's ID"));
            return;
        }
    }
}

TSharedPtr<FMatchmakingAlgorithmStepStateJoinServerSession> FMatchmakingAlgorithmStepJoinServerSession::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateJoinServerSession>();
}

void FMatchmakingAlgorithmStepJoinServerSession::Start(TSharedRef<FMatchmakingEngineTask> Task, FString SessionId)
{
    if (auto Pins = Task->PinAndContinue())
    {
        if (!Pins->OnlineSession.IsValid())
        {
            this->RestartableError(*Task, TEXT("Unable to find game by session ID: Session interface is not valid"));
            return;
        }

        UE_LOG(LogMatchmaking, Verbose, TEXT("%sSearching for session: %s"), *this->GetLogPrefix(*Task), *SessionId);

        if (!Pins->OnlineSession->FindSessionById(
                *Task->Request.UserId,
                *Pins->OnlineSession->CreateSessionIdFromString(SessionId),
                *Task->Request.UserId,
                FOnSingleSessionResultCompleteDelegate::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepJoinServerSession::OnSessionFindComplete,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    SessionId,
                    1)))
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to find game by session ID: Call to FindSessionById failed, try setting "
                     "bConnectViaSessionID to false"));
            return;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()