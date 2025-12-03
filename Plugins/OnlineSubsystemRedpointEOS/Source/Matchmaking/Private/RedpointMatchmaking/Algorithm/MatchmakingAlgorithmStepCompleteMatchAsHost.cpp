// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchAsHost.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchFireEvent.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingLobby.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepFindDedicatedServerSession.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepStartListenServer.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#include "RedpointMatchmaking/MatchmakingSerialization.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateCompleteMatchAsHost> FMatchmakingAlgorithmStepCompleteMatchAsHost::
    CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateCompleteMatchAsHost>();
}

void FMatchmakingAlgorithmStepCompleteMatchAsHost::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        auto Result = MakeShared<FMatchmakingEngineResponse>();
        Result->Teams = FMatchmakingSerialization::DeserializeResultFromLobbyData(
            Pins->OnlineLobby.ToSharedRef(),
            Pins->OnlineIdentity.ToSharedRef(),
            *Task->Request.UserId,
            *Task->MatchmakingLobbyId,
            MatchmakingMetadataResult);
        Task->StoredResponse = Result;

        FMatchmakingSerialization::DumpTeamArrayState(Task->StoredResponse->Teams, TEXT("CompleteMatchAsHost"));

        switch (Task->Request.CompletionBehaviour.Type)
        {
        case EMatchmakingEngineCompletionBehaviourType::ReturnResults:
            this->MoveToStep(*Task, MMSTEP_DELETE_MATCHMAKING_LOBBY, MMSTEP_COMPLETE_MATCH_FIRE_EVENT);
            break;
        case EMatchmakingEngineCompletionBehaviourType::StartListenServerWithMap:
            Task->Request.CompletionBehaviour.OnResultsReady.ExecuteIfBound(
                Task->TaskId,
                Task->Request,
                *Task->StoredResponse);
            this->MoveToStep(*Task, MMSTEP_START_LISTEN_SERVER);
            break;
        case EMatchmakingEngineCompletionBehaviourType::FindDedicatedServer:
            Task->Request.CompletionBehaviour.OnResultsReady.ExecuteIfBound(
                Task->TaskId,
                Task->Request,
                *Task->StoredResponse);
            this->MoveToStep(*Task, MMSTEP_FIND_DEDICATED_SERVER_SESSION);
            break;
        default:
            checkf(false, TEXT("Unsupported CompletionBehaviour.Type"));
            break;
        }
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()