// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEventHandlerFinalization.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../Algorithm/MatchmakingAlgorithmStep.h"
#include "../Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestReject.h"
#include "../Algorithm/MatchmakingAlgorithmStepIncomingFollowRequestVerify.h"
#include "../MatchmakingFollowRequest.h"
#include "../MatchmakingTeamManager.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMHANDLER_FINALIZATION FName("EventHandlerFinalization")

void FMatchmakingEventHandlerFinalization::OnMemberUpdate(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    TSharedRef<FMatchmakingEngineImpl> Engine,
    TWeakPtr<FMatchmakingEngineTask> TaskWk)
{
    auto Task = TaskWk.Pin();
    if (!Task.IsValid())
    {
        return;
    }

    EMIT_TASK_COUNTER_TRACE(Task);

    if (!Task->MatchmakingLobbyId.IsValid() || !Task->Request.UserId.IsValid())
    {
        return;
    }

    if (*Task->MatchmakingLobbyId != LobbyId || *Task->Request.UserId != UserId)
    {
        return;
    }

    auto Step = Engine->GetRegisteredStep<IMatchmakingAlgorithmStep<IMatchmakingAlgorithmStepState>>(Task->CurrentStep);

    if (auto Pins = Task->PinAndContinue())
    {
        EMIT_TASK_COUNTER_TRACE(Task);

        if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_FINALIZED_RESULT) &&
            Task->MatchmakingLobbyId.IsValid())
        {
            FVariantData FinalizedData;
            if (Pins->OnlineLobby->GetMemberMetadataValue(
                    UserId,
                    *Task->MatchmakingLobbyId,
                    MemberId,
                    MatchmakingMemberMetadataFinalized,
                    FinalizedData))
            {
                if (FinalizedData.GetType() == EOnlineKeyValuePairDataType::Bool)
                {
                    bool bIsFinalized;
                    FinalizedData.GetValue(bIsFinalized);
                    if (bIsFinalized)
                    {
                        FName FinalizedKey = FName(*FString::Printf(TEXT("finalized.%s"), *MemberId.ToString()));
                        if (Task->EventData.Contains(FinalizedKey))
                        {
                            // We already handled this finalization event.
                        }
                        else
                        {
                            Task->EventData.Add(FinalizedKey, true);

                            verify(Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_FINALIZED_RESULT));
                        }
                    }
                }
            }
        }

        // Task->MatchmakingLobbyId can be cleared from Step->HandleInterrupt above.
        if (Step->CanHandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_CONNECTING_STARTED) &&
            Task->MatchmakingLobbyId.IsValid())
        {
            FVariantData ConnectingData;
            if (Pins->OnlineLobby->GetMemberMetadataValue(
                    UserId,
                    *Task->MatchmakingLobbyId,
                    MemberId,
                    MatchmakingMemberMetadataConnecting,
                    ConnectingData))
            {
                if (ConnectingData.GetType() == EOnlineKeyValuePairDataType::Bool)
                {
                    bool bIsConnecting;
                    ConnectingData.GetValue(bIsConnecting);
                    if (bIsConnecting)
                    {
                        FName ConnectingKey = FName(*FString::Printf(TEXT("connecting.%s"), *MemberId.ToString()));
                        if (Task->EventData.Contains(ConnectingKey))
                        {
                            // We already handled this connecting event.
                        }
                        else
                        {
                            Task->EventData.Add(ConnectingKey, true);

                            verify(Step->HandleInterrupt(Task.ToSharedRef(), MMINTERRUPT_CLIENT_CONNECTING_STARTED));
                        }
                    }
                }
            }
        }
    }
}

void FMatchmakingEventHandlerFinalization::RegisterEvents(FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        Task.EventHandlers.Add(
            MMHANDLER_FINALIZATION,
            Pins->OnlineLobby->AddOnMemberUpdateDelegate_Handle(FOnMemberUpdateDelegate::CreateSP(
                this,
                &FMatchmakingEventHandlerFinalization::OnMemberUpdate,
                Engine.AsShared(),
                TWeakPtr<FMatchmakingEngineTask>(Task.AsShared()))));
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

void FMatchmakingEventHandlerFinalization::UnregisterEvents(
    FMatchmakingEngineImpl &Engine,
    FMatchmakingEngineTask &Task)
{
    if (auto Pins = Task.PinAndContinue())
    {
        if (Task.EventHandlers.Contains(MMHANDLER_FINALIZATION))
        {
            Pins->OnlineLobby->ClearOnMemberUpdateDelegate_Handle(Task.EventHandlers[MMHANDLER_FINALIZATION]);
            Task.EventHandlers.Remove(MMHANDLER_FINALIZATION);
        }
        EMIT_TASK_COUNTER_TRACE(Task.AsShared());
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()