// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearching.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Containers/Ticker.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFollowHost.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearching.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSearchingMoveToSearchingStatus.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerReceiveFollowRequest.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateSearching> FMatchmakingAlgorithmStepSearching::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateSearching>();
}

void FMatchmakingAlgorithmStepSearching::OnDelayComplete(
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    auto State = this->GetState(*Task);
    if (State->bInterruptedWithFollowRequest)
    {
        // We are no longer the current step.
        return;
    }

    if (!Task->CurrentStep.IsEqual(MMSTEP_SEARCHING))
    {
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);
    this->MoveToStep(*Task, MMSTEP_SEARCHING_MOVE_TO_SEARCHING_STATUS);
}

void FMatchmakingAlgorithmStepSearching::Start(TSharedRef<FMatchmakingEngineTask> Task, bool bIsInitialEntry)
{
    if (bIsInitialEntry)
    {
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_SEARCHING_MOVE_TO_SEARCHING_STATUS);
        return;
    }

    auto State = this->GetState(*Task);
    State->bInterruptedWithFollowRequest = false;

    // Ensure our state is correct before we start the searching process. We should never be searching if we're
    // waiting for users to join.
    check(!Task->TeamManager->HasPendingBatch());

    // Determine how long we should wait for.
    float SecondsDelay = 0.0f;

    // Add some variability into the search frequency, which helps deal with cases where there are a lot of clients
    // who would otherwise have similar alignments on search operations.
    SecondsDelay =
        FMath::RandRange(MatchmakingTimingSearchingVarianceLowerDelay, MatchmakingTimingSearchingVarianceUpperDelay);

    // Check the number of times we were asked to follow and had to reject due to not being in a searching status. This
    // is a logarithmic value to make us wait quite a bit when we get one request in, but not wait forever if we get
    // several.
    if (Task->RejectedRequestsSinceLastSearch > 0)
    {
        SecondsDelay += (float)(FMath::Log2(static_cast<double>(Task->RejectedRequestsSinceLastSearch + 1)) *
                                static_cast<double>(FMath::RandRange(
                                    MatchmakingTimingSearchingPerRejectionLowerDelay,
                                    MatchmakingTimingSearchingPerRejectionUpperDelay)));
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sRejected %d follow requests since last search, delaying this search by %f seconds to ensure this "
                 "lobby is available to other lobbies."),
            *this->GetLogPrefix(*Task),
            Task->RejectedRequestsSinceLastSearch,
            static_cast<double>(SecondsDelay));
        Task->RejectedRequestsSinceLastSearch = 0;
    }

    // Make sure we will at least wait 1 second, to prevent spamming the backend with requests. Additive here so that
    // we avoid having everyone set on the same 1 second delay.
    if (SecondsDelay < MatchmakingTimingSearchingMinimumDelay)
    {
        SecondsDelay += MatchmakingTimingSearchingMinimumDelay;
    }

    // Schedule the search.
    Task->Schedule.Execute(
        SecondsDelay,
        FMatchmakingEngineTaskScheduleCallback::CreateSP(
            this,
            &FMatchmakingAlgorithmStepSearching::OnDelayComplete,
            TWeakPtr<FMatchmakingEngineTask>(Task)));
}

bool FMatchmakingAlgorithmStepSearching::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    return InterruptType.IsEqual(MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST);
}

bool FMatchmakingAlgorithmStepSearching::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_WANT_TO_ACCEPT_FOLLOW_REQUEST))
    {
        auto State = this->GetState(*Task);
        State->bInterruptedWithFollowRequest = true;
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sSearch operation interrupted by follow request for lobby %s"),
            *this->GetLogPrefix(*Task),
            *Task->MatchmakingLobbyId->ToString());
        return true;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()