// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Containers/Ticker.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#include "RedpointMatchmaking/MatchmakingTeamManager.h"
#include "RedpointMatchmaking/Utility/LobbyIdContainers.h"
#include "UObject/NameTypes.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

DECLARE_DELEGATE(FMatchmakingEngineTaskScheduleCallback);
DECLARE_DELEGATE_RetVal_TwoParams(
    FDelegateHandle,
    FMatchmakingEngineTaskSchedule,
    float /* Delay */,
    FMatchmakingEngineTaskScheduleCallback /* Callback */);
DECLARE_DELEGATE_OneParam(FMatchmakingEngineTaskUnschedule, FDelegateHandle /* Handle */);

struct FMatchmakingEngineTaskIncomingFollowRequest
{
public:
    FMatchmakingEngineTaskIncomingFollowRequest(
        TSharedRef<const FUniqueNetId> InFromMemberId,
        TSharedRef<const FOnlineLobbyId> InTargetLobbyId,
        int32 InTargetRequestId,
        int64 InTargetLobbyCreatedUnixTimestamp,
        FString InTargetStateHash)
        : FromMemberId(MoveTemp(InFromMemberId))
        , TargetLobbyId(MoveTemp(InTargetLobbyId))
        , TargetRequestId(InTargetRequestId)
        , TargetLobbyCreatedUnixTimestamp(InTargetLobbyCreatedUnixTimestamp)
        , TargetStateHash(MoveTemp(InTargetStateHash))
    {
    }

    /**
     * The ID of the member that sent the request to us.
     */
    TSharedRef<const FUniqueNetId> FromMemberId;

    /**
     * The target lobby ID that they want us to join.
     */
    TSharedRef<const FOnlineLobbyId> TargetLobbyId;

    /**
     * The target request ID.
     */
    int32 TargetRequestId;

    /**
     * The timestamp that the target lobby was created, as reported by the member.
     */
    int64 TargetLobbyCreatedUnixTimestamp;

    /**
     * The state hash that the requestor observed when sending the request.
     */
    FString TargetStateHash;
};

class FMatchmakingEngineTask : public TSharedFromThis<FMatchmakingEngineTask>
{
public:
    FMatchmakingEngineTask(const FMatchmakingEngineRequest &InRequest, FMatchmakingEngineRequestHandle InHandle);
    UE_NONCOPYABLE(FMatchmakingEngineTask);

    /**
     * The unique task ID, which is prepended to every log message. This is randomly generated when the task is
     * created, and the matchmaking engine will emit a log that allows you to map task IDs back to request IDs.
     */
    const FString TaskId;

    /**
     * The request configuration.
     */
    const FMatchmakingEngineRequest Request;

    /**
     * The handle for the matchmaking operation. This can be used to identify the particular request
     * or to cancel it with the engine.
     */
    const FMatchmakingEngineRequestHandle Handle;

    /**
     * The state for each algorithm step for this specific task.
     */
    TMap<FName, TSharedPtr<class IMatchmakingAlgorithmStepState>> StepStates;

    /**
     * The name of the current step.
     */
    FName CurrentStep;

    /**
     * Manages the team state for this task.
     */
    TSharedPtr<FMatchmakingEngineTaskTeamManager> TeamManager;

    /**
     * The lobby that is used to receive matchmaking follow requests.
     */
    TSharedPtr<const FOnlineLobbyId> MatchmakingFollowRequestLobbyId;

    /**
     * The lobby that is used for matchmaking.
     */
    TSharedPtr<const FOnlineLobbyId> MatchmakingLobbyId;

    /**
     * Lobbies which are ignored in search results. This is used to exclude lobbies that we failed to
     * connect to on subsequent searches.
     */
    TLobbyIdSet ExcludedLobbyIds;

    /**
     * The candidate evaluator that determines suitable match candidates.
     */
    TSharedRef<class IMatchmakingCandidateEvaluator> CandidateEvaluator;

    /**
     * When event handlers register delegates, they are registered per task and need to be unregistered when the task
     * completes. This field is used to store the FDelegateHandle values.
     */
    TMap<FName, FDelegateHandle> EventHandlers;

    /**
     * Arbitrary data storage for event handlers.
     */
    TMap<FName, FVariantData> EventData;

    /**
     * When sending outbound follow requests, we use an auto-incrementing ID so that the receiver can detect a second
     * request from us (i.e. when we retry the same lobby).
     */
    int32 FollowRequestId;

    /**
     * When a step wants to schedule a callback at some time in the future, it can use this function. In the default
     * behaviour the engine will use this to either schedule it at the given delay using FCoreTicker(). For unit tests,
     * it will ignore the delay and always schedule on the next frame (since the test backend is instant).
     */
    FMatchmakingEngineTaskSchedule Schedule;

    /**
     * When a step wants to cancel a callback it scheduled with Schedule, it can use this function.
     */
    FMatchmakingEngineTaskUnschedule Unschedule;

    /**
     * If this is not FDateTime::MinValue, it's the estimated time that matchmaking will complete.
     */
    FDateTime EstimatedTimeOfCompletion;

    /**
     * Tracks the number of incoming requests that we had to request because we weren't in the right step. The
     * MMSTEP_SEARCHING step uses this value to increase the searching delay if this lobby is in high demand.
     */
    int32 RejectedRequestsSinceLastSearch;

    /**
     * Used by the engine to automatically heartbeat the matchmaking lobby every 5 seconds if we're not operating as a
     * client.
     */
    FTSTicker::FDelegateHandle HeartbeatDelegateHandle;

    /**
     * Tracks the last time we saw a new host heartbeat value come in on the lobby that we are following.
     */
    FDateTime LastHostHeartbeatTime;

    /**
     * Tracks the last value we saw for host heartbeats; this is used to detect when the host heartbeat value has
     * changed.
     */
    int64 LastHostHeartbeatValue;

    /**
     * Tracks when this matchmaking task was started.
     */
    FDateTime Started;

    /**
     * The incoming follow requests for consideration. When a follow request comes in, we put it in this array and then
     * start the follow request timer if it's not already running. When the timer fires, it sorts all of the entries in
     * array with the oldest lobby first, and then we try to verify that. This means that clients will preference older
     * lobbies over newer ones.
     */
    TArray<FMatchmakingEngineTaskIncomingFollowRequest> IncomingFollowRequestsForConsideration;

    /**
     * The delegate handle for the follow request timer, which is started when the first follow request is made.
     */
    FDelegateHandle IncomingFollowRequestDelayTimerHandle;

    /**
     * The number of requests we sent to other hosts that ended up being rejected or timing out.
     */
    int RejectedRequestsPenaltyCount;

    /**
     * The local controller index of the user.
     */
    int32 ControllerIndex;

    /**
     * The response for the completion delegate. This is stored at the task level so that when we're creating listen
     * servers or searching for dedicated servers, we don't have to pass the response through all of those steps.
     */
    TSharedPtr<FMatchmakingEngineResponse> StoredResponse;

    /**
     * Tracks whether the party must be unlocked when matchmaking completes or is cancelled; set by
     * MatchmakingAlgorithmStepLockParty.
     */
    bool PartyMustBeUnlocked;

    /**
     * Dedicated servers which are ignored in search results. This is used to exclude dedicated servers that we failed
     * to reserve on subsequent searches.
     */
    TSet<FString> ExcludedDedicatedServerSessionIds;

    /**
     * Indicates whether this task is responsible for deleting the matchmaking lobby after successfully connecting to a
     * game server.
     */
    bool bResponsibleForLobbyDeletionStep;

    struct FMatchmakingEngineTaskPins
    {
    public:
        /**
         * The reference to the IOnlineIdentity interface.
         */
        // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
        TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> OnlineIdentity;

        /**
         * The reference to the IOnlineLobby interface.
         */
        // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
        TSharedPtr<IOnlineLobby, ESPMode::ThreadSafe> OnlineLobby;

        /**
         * The reference to the IOnlinePartySystem interface.
         */
        // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
        TSharedPtr<IOnlinePartySystem, ESPMode::ThreadSafe> OnlineParty;

        /**
         * The reference to the IOnlineSession interface.
         */
        // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
        TSharedPtr<IOnlineSession, ESPMode::ThreadSafe> OnlineSession;

        /**
         * The reference to the IOnlineStats interface.
         */
        // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
        TSharedPtr<IOnlineStats, ESPMode::ThreadSafe> OnlineStats;
    };

    /**
     * Returns the pinned references if this task should be processed.
     */
    TSharedPtr<FMatchmakingEngineTaskPins> PinAndContinue() const;
};

#define EMIT_TASK_COUNTER_TRACE(Task)
/*
#define EMIT_TASK_COUNTER_TRACE(Task)                                                                                  \
    UE_LOG(                                                                                                            \
        LogMatchmaking,                                                                                                \
        Verbose,                                                                                                       \
        TEXT("Task %s: [Trace] At " __FILE__ ":" PREPROCESSOR_TO_STRING(__LINE__) " had %d references."),              \
        *(Task)->TaskId,                                                                                               \
        (Task).GetSharedReferenceCount())
*/

#endif

REDPOINT_EOS_CODE_GUARD_END()