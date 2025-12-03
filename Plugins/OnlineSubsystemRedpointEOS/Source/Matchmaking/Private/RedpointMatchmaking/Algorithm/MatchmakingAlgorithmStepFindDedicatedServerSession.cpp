// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFindDedicatedServerSession.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlineSessionInterface.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepReserveDedicatedServerForMatch.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerAbandonment.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

void FMatchmakingAlgorithmStepFindDedicatedServerSession::OnFindSessionsComplete(
    bool bWasSuccessful,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    TSharedRef<FOnlineSessionSearch> Search,
    int32 AttemptNumber)
{
    auto Task = TaskWk.Pin();
    check(Task.IsValid());

    EMIT_TASK_COUNTER_TRACE(Task);

    UE_LOG(LogMatchmaking, Verbose, TEXT("%sGot FindSessions callback"), *this->GetLogPrefix(*Task));

    if (!Task->CurrentStep.IsEqual(MMSTEP_FIND_DEDICATED_SERVER_SESSION))
    {
        return;
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (!Pins->OnlineSession.IsValid())
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to search for dedicated server sessions: Session interface is not valid"));
            return;
        }

        auto State = this->GetState(*Task);

        if (!bWasSuccessful)
        {
            Pins->OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(State->FindSessionsDelegateHandle);
            this->RestartableError(*Task, TEXT("Unable to search for dedicated server sessions: Search failed"));
            return;
        }

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sAttempt #%d: Received %d search results."),
            *this->GetLogPrefix(*Task),
            AttemptNumber,
            Search->SearchResults.Num());

        for (const auto &SearchResult : Search->SearchResults)
        {
            if (!SearchResult.IsSessionInfoValid())
            {
                // Ignored because this is an invalid session result.
                continue;
            }

            FString SearchResultSessionId = SearchResult.GetSessionIdStr();

            if (Task->ExcludedDedicatedServerSessionIds.Contains(SearchResultSessionId))
            {
                // Ignored because we previously tried this server and failed to reserve it.
                continue;
            }

            FName Port = Task->Request.CompletionBehaviour.DedicatedServerBeaconPort;
            if (Port.IsNone())
            {
                Port = FName(*FString::Printf(TEXT("%d"), MatchmakingDedicatedServerDefaultPort));
            }

            FString BeaconConnectInfo;
            FString GameConnectInfo;
            if (!Pins->OnlineSession->GetResolvedConnectString(SearchResult, NAME_GamePort, GameConnectInfo) ||
                !Pins->OnlineSession->GetResolvedConnectString(SearchResult, Port, GameConnectInfo))
            {
                // We can't get the required ports for this server, ignore it.
                continue;
            }

            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sAttempt #%d: Selected dedicated server %s for reservation attempt."),
                *this->GetLogPrefix(*Task),
                AttemptNumber,
                *SearchResultSessionId);
            Pins->OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(State->FindSessionsDelegateHandle);
            this->MoveToStep(*Task, MMSTEP_RESERVE_DEDICATED_SERVER_FOR_MATCH, SearchResult);
            return;
        }

        // We didn't get any suitable results. Search again in a few seconds.
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sAttempt #%d: No suitable dedicated servers found in search results, searching again in 5 "
                 "seconds..."),
            *this->GetLogPrefix(*Task),
            AttemptNumber);
        Pins->OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(State->FindSessionsDelegateHandle);
        State->FindSessionsDelegateHandle.Reset();
        State->ScheduledRetryHandle = FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(
                this,
                &FMatchmakingAlgorithmStepFindDedicatedServerSession::SearchForDedicatedServerSession,
                TaskWk,
                AttemptNumber),
            5.0f);
    }
}

bool FMatchmakingAlgorithmStepFindDedicatedServerSession::SearchForDedicatedServerSession(
    float DeltaSeconds,
    TWeakPtr<FMatchmakingEngineTask> TaskWk,
    int32 AttemptNumber)
{
    auto Task = TaskWk.Pin();
    if (Task.IsValid())
    {
        EMIT_TASK_COUNTER_TRACE(Task);

        if (!Task->CurrentStep.IsEqual(MMSTEP_FIND_DEDICATED_SERVER_SESSION))
        {
            return false;
        }

        if (auto Pins = Task->PinAndContinue())
        {
            if (!Pins->OnlineSession.IsValid())
            {
                this->RestartableError(
                    *Task,
                    TEXT("Unable to search for available dedicated servers: Session interface is not valid"));
                return false;
            }

            UE_LOG(
                LogMatchmaking,
                Verbose,
                TEXT("%sAttempt #%d: Searching for dedicated servers"),
                *this->GetLogPrefix(*Task),
                AttemptNumber);

            TSharedRef<FOnlineSessionSearch> Search = MakeShared<FOnlineSessionSearch>();
            Search->bIsLanQuery = false;
            Search->MaxSearchResults = 10;
            Search->PingBucketSize = 50;
            Search->PlatformHash = 0;
            if (Task->Request.CompletionBehaviour.OnGetDedicatedServerSearchParams.IsBound())
            {
                // Replace search parameters.
                Search->QuerySettings.SearchParams =
                    Task->Request.CompletionBehaviour.OnGetDedicatedServerSearchParams
                        .Execute(Task->TaskId, Task->Request, *Task->StoredResponse, 1);
            }
            else
            {
                // Remove default search parameters.
                Search->QuerySettings.SearchParams.Empty();
            }
            Search->SearchState = EOnlineAsyncTaskState::NotStarted;
            Search->TimeoutInSeconds = 10.0f;

            auto State = this->GetState(*Task);
            State->ScheduledRetryHandle.Reset();
            State->FindSessionsDelegateHandle =
                Pins->OnlineSession->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateSP(
                    this,
                    &FMatchmakingAlgorithmStepFindDedicatedServerSession::OnFindSessionsComplete,
                    TWeakPtr<FMatchmakingEngineTask>(Task),
                    Search,
                    AttemptNumber));

            if (!Pins->OnlineSession->FindSessions(*Task->Request.UserId, Search))
            {
                Pins->OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(State->FindSessionsDelegateHandle);
                this->RestartableError(*Task, TEXT("Unable to find dedicated server: Call to FindSessions failed"));
                return false;
            }
        }
    }

    // Don't renew ticker.
    return false;
}

TSharedPtr<FMatchmakingAlgorithmStepStateFindDedicatedServerSession>
FMatchmakingAlgorithmStepFindDedicatedServerSession::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateFindDedicatedServerSession>();
}

void FMatchmakingAlgorithmStepFindDedicatedServerSession::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        if (!Pins->OnlineSession.IsValid())
        {
            this->RestartableError(
                *Task,
                TEXT("Unable to search for available dedicated servers: Session interface is not valid"));
            return;
        }

        this->SearchForDedicatedServerSession(0.0f, Task, 1);
    }
}

bool FMatchmakingAlgorithmStepFindDedicatedServerSession::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT))
    {
        // We have special cleanup to do if we need to abandon.
        return true;
    }

    return false;
}

bool FMatchmakingAlgorithmStepFindDedicatedServerSession::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        // Need to unregister the global FindSessions handler we registered, if we can.
        if (auto Pins = Task->PinAndContinue())
        {
            auto State = this->GetState(*Task);
            if (Pins->OnlineSession.IsValid())
            {
                if (State->FindSessionsDelegateHandle.IsValid())
                {
                    UE_LOG(
                        LogMatchmaking,
                        Verbose,
                        TEXT("%sCleared globally registered FindSessions handler before abandoning"),
                        *this->GetLogPrefix(*Task));
                    Pins->OnlineSession->ClearOnFindSessionsCompleteDelegate_Handle(State->FindSessionsDelegateHandle);
                    State->FindSessionsDelegateHandle.Reset();
                }

                if (State->ScheduledRetryHandle.IsValid())
                {
                    UE_LOG(
                        LogMatchmaking,
                        Verbose,
                        TEXT("%sCleared pending retry handle before abandoning"),
                        *this->GetLogPrefix(*Task));
                    FTSTicker::GetCoreTicker().RemoveTicker(State->ScheduledRetryHandle);
                    State->ScheduledRetryHandle.Reset();
                }
            }
        }

        // We're not handling the abandonment, tell the event handler it still needs to move to the abandon step.
        return false;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()