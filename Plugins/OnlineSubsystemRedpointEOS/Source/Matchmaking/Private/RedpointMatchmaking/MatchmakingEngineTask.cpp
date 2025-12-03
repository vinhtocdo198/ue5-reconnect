// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingEngineTask.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "MatchmakingCandidateEvaluatorDefault.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

static FString GenerateRandomTaskId()
{
    FString Chars = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjklmnpqrstuvwxyz1234567890";
    FString TaskId = "";
    for (int i = 0; i < 8; i++)
    {
        TaskId += Chars.GetCharArray()[FMath::RandRange(0, Chars.Len() - 1)];
    }
    return TaskId;
}

FMatchmakingEngineTask::FMatchmakingEngineTask(
    const FMatchmakingEngineRequest &InRequest,
    FMatchmakingEngineRequestHandle InHandle)
    : TaskId(GenerateRandomTaskId())
    , Request(InRequest)
    , Handle(InHandle)
    , StepStates(TMap<FName, TSharedPtr<class IMatchmakingAlgorithmStepState>>())
    , CurrentStep(NAME_None)
    , TeamManager(
          InRequest.HostConfiguration.IsValid()
              ? TSharedPtr<FMatchmakingEngineTaskTeamManager>(MakeShared<FMatchmakingEngineTaskTeamManager>(InRequest))
              : nullptr)
    , MatchmakingFollowRequestLobbyId(nullptr)
    , MatchmakingLobbyId(nullptr)
    , ExcludedLobbyIds(TLobbyIdSet())
    , CandidateEvaluator(MakeShared<FMatchmakingCandidateEvaluatorDefault>())
    , EventHandlers(TMap<FName, FDelegateHandle>())
    , EventData(TMap<FName, FVariantData>())
    , FollowRequestId(0)
    , Schedule()
    , Unschedule()
    , EstimatedTimeOfCompletion(FDateTime::MinValue())
    , RejectedRequestsSinceLastSearch(0)
    , HeartbeatDelegateHandle()
    , LastHostHeartbeatTime(FDateTime::UtcNow())
    , LastHostHeartbeatValue(0)
    , Started(FDateTime::UtcNow())
    , IncomingFollowRequestsForConsideration()
    , IncomingFollowRequestDelayTimerHandle()
    , RejectedRequestsPenaltyCount(0)
    , ControllerIndex()
    , StoredResponse(nullptr)
    , PartyMustBeUnlocked(false)
    , ExcludedDedicatedServerSessionIds()
    , bResponsibleForLobbyDeletionStep(false)
{
    if (!Request.Session.IsValid())
    {
        if (Request.CompletionBehaviour.Type == EMatchmakingEngineCompletionBehaviourType::StartListenServerWithMap)
        {
            checkf(false, TEXT("The request requires a session interface, but one was not set!"));
        }
    }
    if (Request.WorldContextHandle.IsNone())
    {
        if (Request.CompletionBehaviour.Type == EMatchmakingEngineCompletionBehaviourType::StartListenServerWithMap)
        {
            checkf(false, TEXT("The request requires a world context name, but one was not set!"));
        }
    }

    if (this->Request.HostConfiguration.IsValid())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("Task %s: This task is a host task that maps to request ID %lld"),
            *this->TaskId,
            this->Request.HostConfiguration->RequestId);
    }
    else
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("Task %s: This task is a client task"), *this->TaskId);
    }
}

TSharedPtr<FMatchmakingEngineTask::FMatchmakingEngineTaskPins> FMatchmakingEngineTask::PinAndContinue() const
{
    if (this->Request.Identity.IsValid() && this->Request.Lobby.IsValid())
    {
        TSharedPtr<FMatchmakingEngineTaskPins> Pins = MakeShared<FMatchmakingEngineTaskPins>();
        Pins->OnlineIdentity = this->Request.Identity.Pin();
        Pins->OnlineLobby = this->Request.Lobby.Pin();
        Pins->OnlineParty = this->Request.PartySystem.Pin();
        Pins->OnlineSession = this->Request.Session.Pin();
        Pins->OnlineStats = this->Request.Stats.Pin();
        return Pins;
    }

    return nullptr;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()