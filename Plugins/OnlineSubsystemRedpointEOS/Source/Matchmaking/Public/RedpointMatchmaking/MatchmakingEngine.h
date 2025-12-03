// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Delegates/DelegateCombinations.h"
#include "Online/CoreOnline.h"
#include "OnlineError.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemTypes.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

enum class EMatchmakingEngineResponseTeamSlotType : uint8
{
    /** A player. */
    User,

    /** An AI filled slot. */
    AI,

    /** An empty slot. */
    Empty,
};

struct MATCHMAKING_API FMatchmakingEngineResponseTeamSlot
{
    /** What is filling this slot? */
    EMatchmakingEngineResponseTeamSlotType Type;

    /** If this slot is filled by a player, this is their user ID. */
    TSharedPtr<const FUniqueNetId> UserId;
};

struct MATCHMAKING_API FMatchmakingEngineResponseTeam
{
    /**
     * The slots in the team, including the user IDs and type.
     */
    TArray<FMatchmakingEngineResponseTeamSlot> Slots;
};

struct MATCHMAKING_API FMatchmakingEngineResponse
{
    /**
     * The teams in the match.
     */
    TArray<FMatchmakingEngineResponseTeam> Teams;
};

enum class EMatchmakingBehaviourOnNoCandidates : uint8
{
    WaitUntilFull,
    CompletePartiallyFilled,
    CompleteFillWithAI,
};

enum class EMatchmakingBalanceMode : uint8
{
    /** Maximizes team fill by putting new members into the teams with the fewest slots left. This mode increases the
    chance of having a full match. */
    MaximizeTeamFill,

    /** Maximizes team balance by putting new members into the teams with the most slots left. This favours having
    balanced teams over a full match. */
    MaximizeBalance,
};

struct MATCHMAKING_API FMatchmakingSkillThresholdFunctionContext
{
public:
    /**
     * The number of seconds until matchmaking would complete due to a timeout
     * (under a "partial fill" mode). If matchmaking is configured to wait forever,
     * this still represents the seconds left until reaching the timeout, though
     * matchmaking continues after the timeout. For matchmaking after the timeout,
     * this value will be zero (it can't be negative).
     */
    float SecondsRemaining;

    /**
     * Based on the number of unfilled slots, contains the maximum number of
     * seconds remaining that could be in the "SecondsRemaining" field. This allows
     * you to compute the "percentage seconds remaining" with:
     *
     * SecondsRemaining / IterationMaximumSecondsRemaining
     */
    float IterationMaximumSecondsRemaining;
};

DECLARE_DELEGATE_RetVal_OneParam(
    double /* MaximumBalanceDifferential */,
    FMatchmakingSkillThresholdFunction,
    const FMatchmakingSkillThresholdFunctionContext & /* ThresholdContext */);

DECLARE_DELEGATE_FourParams(
    FMatchmakingEngineRequestCandidateRejected,
    const FString & /* TaskId */,
    const struct FMatchmakingEngineRequest & /* Request */,
    const FString & /* CandidateLobbyId */,
    const FString & /* CandidateRejectionReason */);

struct MATCHMAKING_API FMatchmakingEngineRequestHostConfiguration
{
public:
    /**
     * The name of the matchmaking queue.
     */
    FString QueueName;

    /**
     * A semi-unique request ID. This field is used to ensure that hosts never request each other (they will only
     * consider candidiate lobbies where the target request ID is higher than this field). Usually you will want to make
     * this the unix timestamp (UTC) of when the request was scheduled, but it's exposed here because the unit tests
     * need to set it in a deterministic way.
     */
    int64 RequestId;

    /**
     * Each array entry is a new team, and each value specifies the number of members in that team.
     */
    TArray<int32> TeamCapacities;

    /**
     * When there are no more candidate matches, and the minimum wait seconds has been reached, how should the
     * matchmaker complete the match?
     */
    EMatchmakingBehaviourOnNoCandidates OnNoCandidates;

    /**
     * When the matchmaker is configured to accept partially filled matches, this specifies the minimum number of team
     * members on each team that must be met for a partial fill to be accepted. You can use this to guarantee that, even
     * for partial fills, each team will still have a minimum number of players on it.
     */
    int MinimumTeamMemberCountForPartialFill;

    /**
     * The baseline number of seconds to wait when there are no more candidate matches. This value must be at least 1,
     * but you should probably set a value like 30 seconds here (the lower this value, the more likely that the match
     * will be partially filled).
     */
    int MinimumWaitSecondsBaseline;

    /**
     * The number of wait seconds to add, per remaining empty slot. This allows you to wait longer for more players when
     * the match is more empty. For example, if the baseline is set to 30 seconds, and the per empty slot is 5 seconds,
     * then the wait time on no candidate matches will be as follows:
     *
     * 1 empty slot:  35 seconds
     * 2 empty slots: 40 seconds
     * 3 empty slots: 45 seconds
     * ... and so on ...
     */
    int MinimumWaitSecondsPerEmptySlot;

    /**
     * The number of wait seconds to be deducted for each rejected follow request. The wait seconds counter resets when
     * searching starts again after a follow request (i.e. the wait seconds are not for the total matchmaking process).
     * This means if you consistently find candidates right at the end of the waited search time, but they reject you,
     * you'll constantly reset to a high wait time after each rejection.
     *
     * The penalty seconds are deducted from the starting wait time for each rejection you get; this means that if
     * you're not making progress with the allowed candidates you do find, you'll eventually reach the OnNoCandidates
     * behaviour. The counted penalties are also retained across internal restarts.
     *
     * This setting is not typically required in a player base where all of the clients are well-behaved. But if you
     * have a lot of players Alt-F4'ing the game or cancelling matchmaking, this can help ensure that clients quickly
     * get into matches even if they are partial fills.
     */
    int PenaltyTimeSecondsPerRejectedRequest;

    /**
     * Determines the placement of players when performing matchmaking.
     */
    EMatchmakingBalanceMode BalanceMode;

    /**
     * If this isn't set to empty string, skill-based matchmaking is turned on, and this is the stats prefix used for
     * skill-based matchmaking. You must also provide the "Stats" online subsystem interface in the request.
     *
     * When skill-based matchmaking is turned on, hosts will check the <prefix>_mu and <prefix>_sigma stats for
     * players in matchmaking to optimally match players using Openskill.
     */
    FString SkillStatPrefix;

    /**
     * If set, this function should return the maximum tolerable balance differential for candidate matches. When
     * skill-based matchmaking is evaluating candidates, it checks the new proposed team layout based on the skill
     * levels, and determines the probability of each team winning. The value returned by this function is the
     * maximum allowable difference in probabilities between any two teams in the prediction.
     *
     * That is, if this function were to return 25.0, then the predicted probabilities of winning for a proposed
     * two-team layout would be "37.5% vs 62.5%". If the predicted probability would be "36% vs 64%", then the
     * candidate would be excluded because it would put the match "too out of balance".
     *
     * Since the returned value from this function is considered the upper-bound, when scoring viable candidates,
     * it uses a linear gradient between "perfectly balanced" and "upper bound". The closer a candidate is to making
     * the match perfectly balanced, the higher it scores and the more likely it is to be selected as the candidate
     * on the given matchmaking iteration. Thus, the algorithm leans towards more balanced matches.
     *
     * This function receives the "seconds remaining" timeout as part of the context; it's expected that you increase
     * the permitted balance differential as the matchmaking process runs out of time. That means the longer
     * matchmaking continues without finding a suitable candidate, the more likely you are to tolerate an unbalanced
     * match. If you want matchmaking to prefer unbalanced matches over partial matches, you'll want to eventually
     * return 100.0 from this function (which means it will consider any candidate, regardless of balance).
     *
     * If you don't set a function here, it defaults to:
     *
     * 10.0 + (1.0 - FMath::Max(0.0, (SecondsRemaining / (IterationMaximumSecondsRemaining - 5.0)))) * 90.0
     */
    FMatchmakingSkillThresholdFunction SkillThresholdFunction;

    /**
     * This delegate is fired when a search rejects a candidate. This is primarily used for automation testing.
     */
    FMatchmakingEngineRequestCandidateRejected OnCandidateRejected;
};

/** A handle for an in-progress matchmaking operation. */
typedef int64 FMatchmakingEngineRequestHandle;

struct MATCHMAKING_API FMatchmakingEngineProgressInfo
{
public:
    /**
     * The handle for the matchmaking operation. This can be used to identify the particular request
     * or to cancel it with the engine.
     */
    FMatchmakingEngineRequestHandle Handle;

    /**
     * The user the operation is for. This matches the UserId in the original request.
     */
    TSharedPtr<const FUniqueNetId> UserId;

    /**
     * The party that the user belongs to. This matches the PartyId in the original request, and will be null if the
     * party was not specified when the request was started.
     */
    TSharedPtr<const FOnlinePartyId> PartyId;

    /**
     * The internal name of the current step.
     */
    FName StepName;

    /**
     * The current overall status of the matchmaking operation.
     */
    FText CurrentStatus;

    /**
     * The detail of the current matchmaking operation (a more detailed version of CurrentStatus).
     */
    FText CurrentDetail;

    /**
     * If the step provides a progress between 0% and 100%, this field will be a value between 0.0f and 1.0f. If the
     * current step does not provide a progress indicator, this field will be less than 0.0f (typically -1.0f).
     */
    float CurrentProgress;

    /**
     * If the step can calculate an estimated time of completion for the matchmaking operation, this field will
     * contain the UTC UNIX timestamp of that time. If an estimate is unavailable, this will be FDateTime::MinValue().
     *
     * The receiver of the event will most likely need to cache this result, as intermittent steps may return
     * FDateTime::MinValue() when they are not adjusting the existing estimate.
     */
    FDateTime EstimatedTimeOfCompletion;
};

DECLARE_DELEGATE_OneParam(FMatchmakingEngineSubmitComplete, const FOnlineError & /* Error */);

DECLARE_DELEGATE_ThreeParams(
    FMatchmakingEngineRequestProgress,
    const FString & /* TaskId */,
    const struct FMatchmakingEngineRequest & /* Request */,
    FMatchmakingEngineProgressInfo /* ProgressInfo */);

DECLARE_DELEGATE_ThreeParams(
    FMatchmakingEngineRequestComplete,
    const FString & /* TaskId */,
    const struct FMatchmakingEngineRequest & /* Request */,
    FMatchmakingEngineResponse /* Response */);

DECLARE_DELEGATE_ThreeParams(
    FMatchmakingEngineRequestResultsReady,
    const FString & /* TaskId */,
    const struct FMatchmakingEngineRequest & /* Request */,
    FMatchmakingEngineResponse /* Response */);

DECLARE_DELEGATE_TwoParams(
    FMatchmakingEngineRequestCancelled,
    const FString & /* TaskId */,
    const struct FMatchmakingEngineRequest & /* Request */);

DECLARE_DELEGATE_ThreeParams(
    FMatchmakingEngineRequestError,
    const FString & /* TaskId */,
    const struct FMatchmakingEngineRequest & /* Request */,
    const FOnlineError & /* Error */);

DECLARE_DELEGATE_RetVal_FourParams(
    FSearchParams,
    FMatchmakingEngineGetDedicatedServerSearchParams,
    const FString & /* TaskId */,
    const struct FMatchmakingEngineRequest & /* Request */,
    FMatchmakingEngineResponse /* Response */,
    int32 /* SearchIteration */);

enum class EMatchmakingEngineCompletionBehaviourType : uint8
{
    /**
     * If this behaviour is selected, the OnComplete delegate is fired with the matchmaking results.
     */
    ReturnResults,

    /**
     * If this behaviour is selected, the leader of the party will start a listen server with the specified map, and the
     * other clients will join it. After the host has notified clients and after clients have joined the server, the
     * OnComplete delegate is fired.
     */
    StartListenServerWithMap,

    /**
     * If this behaviour is selected, the leader of the party searches the sessions list for an available dedicated
     * server, reserves it via the beacon system, and then instructs all of the other match members to join it.
     */
    FindDedicatedServer,
};

struct MATCHMAKING_API FMatchmakingEngineCompletionBehaviour
{
public:
    /**
     * The type of behaviour to execute when the matchmaking process is complete.
     */
    EMatchmakingEngineCompletionBehaviourType Type;

    /**
     * This delegate is fired when the behaviour type is not set to "ReturnResults". It fires as soon as the matchmaking
     * results are ready, before listen servers are started or game server is connected to, and is used to load the
     * results into the cache for the blueprint-based matchmaker.
     *
     * Importantly, this delegate fires before the game will need to choose the starting location of the host player, so
     * if you need to choose player starting locations based on the matchmaking results, and you're using the
     * matchmaking plugin to start the game server, you'll need to bind to this delegate.
     *
     * This delegate is *not* fired for "ReturnResults", since it would fire at the same time as
     * "OnComplete" in that case.
     */
    FMatchmakingEngineRequestComplete OnResultsReady;

    /**
     * This delegate is always fired, but the specific timing of when it is fired depends on the behaviour type.
     */
    FMatchmakingEngineRequestComplete OnComplete;

    /**
     * The name of the map to start when the completion behaviour is StartListenServerWithMap.
     */
    FName MapName;

    /**
     * If true and the completion behaviour is StartListenServerWithMap, the host tells clients to join the session by
     * the session ID, and has them call JoinSession. If false, the host tells the clients the connection address to
     * join directly.
     *
     * This must be set to false when using the Steam backend, as the Steam online subsystem does not support globally
     * searching for a session by it's ID.
     */
    bool bConnectViaSessionID;

    /**
     * The port name that the dedicated server beacon listens on. If this is left unset (NAME_None), it defaults to 9990
     * which is the default setting on the beacon actor. This can either be a port name mapping like NAME_BeaconPort or
     * FName(TEXT("matchmaking")), or it can be a port number directly encapsulated as a name like FName(TEXT("9990")).
     */
    FName DedicatedServerBeaconPort;

    /**
     * If the completion behaviour is set to FindDedicatedServer, this callback is invoked to get the search parameters
     * for finding a dedicated server. You can use the search iteration and response values to tune the
     * search query over time. The search iteration value starts at 1.
     */
    FMatchmakingEngineGetDedicatedServerSearchParams OnGetDedicatedServerSearchParams;
};

struct MATCHMAKING_API FMatchmakingEngineRequest
{
public:
    /**
     * The identity implementation for the online subsystem. You can get this by calling GetIdentityInterface() on an
     * IOnlineSubsystem* pointer. We can't store the IOnlineSubsystem* pointer directly in this request class, because
     * we can't track it's lifetime (and there's no way to get the shared pointer of an online subsystem).
     *
     * The identity interface is required.
     */
    TWeakPtr<class IOnlineIdentity, ESPMode::ThreadSafe> Identity;

    /**
     * The lobby implementation for the online subsystem. You can get this by calling Online::GetLobbyInterface(OSS). We
     * can't store the IOnlineSubsystem* pointer directly in this request class, because we can't track it's lifetime
     * (and there's no way to get the shared pointer of an online subsystem).
     *
     * The lobby interface is required.
     */
    TWeakPtr<class IOnlineLobby, ESPMode::ThreadSafe> Lobby;

    /**
     * The party implementation for the online subsystem. You can get this by calling GetPartyInterface() on an
     * IOnlineSubsystem* pointer. We can't store the IOnlineSubsystem* pointer directly in this request class, because
     * we can't track it's lifetime (and there's no way to get the shared pointer of an online subsystem).
     *
     * The party system is optional; if it's not provided, you can only queue solo players.
     */
    TWeakPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> PartySystem;

    /**
     * The session implementation for the online subsystem. You can get this by calling GetSessionInterface() on an
     * IOnlineSubsystem* pointer. We can't store the IOnlineSubsystem* pointer directly in this request class, because
     * we can't track it's lifetime (and there's no way to get the shared pointer of an online subsystem).
     *
     * The session interface is required if your completion behaviour is not just "return results".
     */
    TWeakPtr<class IOnlineSession, ESPMode::ThreadSafe> Session;

    /**
     * The stats implementation for the online subsystem. You can get this by calling GetStatsInterface() on an
     * IOnlineSubsystem* pointer. We can't store the IOnlineSubsystem* pointer directly in this request class, because
     * we can't track it's lifetime (and there's no way to get the shared pointer of an online subsystem).
     *
     * The stats interface is required to use skill-based matchmaking.
     */
    TWeakPtr<class IOnlineStats, ESPMode::ThreadSafe> Stats;

    /**
     * The user making the request.
     */
    TSharedPtr<const FUniqueNetId> UserId;

    /**
     * The party that the user belongs to. If this is null, then the user will be treated as if they are queuing
     * into matchmaking solo.
     */
    TSharedPtr<const FOnlinePartyId> PartyId;

    /**
     * The world context handle for the current world context. This is required if you are using a completion behaviour
     * other than "ReturnResults", since matchmaking will need a world context in order to load maps and connect to
     * servers.
     */
    FName WorldContextHandle;

    /**
     * If this request should act as a host, this is the host configuration. If this is left as null, then this request
     * will follow the leader of the party or error if that's not possible.
     */
    TSharedPtr<const FMatchmakingEngineRequestHostConfiguration> HostConfiguration;

    /**
     * Configures how the matchmaking process behaves once matchmaking is complete. You use the settings in this
     * structure to define whether the results should just be returned, whether listen servers are started or whether a
     * dedicated server should be found.
     */
    FMatchmakingEngineCompletionBehaviour CompletionBehaviour;

    /**
     * The delegate which will be fired when the matchmaking process progresses.
     */
    FMatchmakingEngineRequestProgress OnProgress;

    /**
     * The delegate which will be fired when the matchmaking process is cancelled.
     */
    FMatchmakingEngineRequestCancelled OnCancelled;

    /**
     * The delegate that will be fired when the matchmaking process encounters a permanent, unrecoverable error.
     * Matchmaking will recover from temporary errors gracefully and most severe errors by transparently restarting the
     * matchmaking process. However, there's a certain class of permanent errors such as an invalid request
     * configuration that there is no way to recover from. This event is only fired for the latter case.
     */
    FMatchmakingEngineRequestError OnError;
};

class MATCHMAKING_API IMatchmakingEngine
{
private:
    static TSharedPtr<IMatchmakingEngine> Instance;

public:
    IMatchmakingEngine() = default;
    UE_NONCOPYABLE(IMatchmakingEngine);
    virtual ~IMatchmakingEngine() = default;

    /**
     * Retrieves the IMatchmakingEngine instance.
     */
    static TSharedPtr<IMatchmakingEngine> Get();

    /**
     * Forcibly creates a new instance of the matchmaking engine instead of retrieving the existing instance. This is
     * used by the unit tests to ensure isolation between different tests, but you should never use this in game code.
     */
    static TSharedPtr<IMatchmakingEngine> ForciblyCreateInstanceForUnitTests();

    /**
     * Enqueues a matchmaking request to the engine, and returns the handle. The OnComplete callback of the request will
     * be called when the request is complete with the results (it will also be called for errors or cancellation).
     */
    virtual FMatchmakingEngineRequestHandle Enqueue(const FMatchmakingEngineRequest Request) = 0;

    /**
     * Cancels a pending matchmaking request identified by the handle. This will cause OnCancelled to be called after
     * the matchmaking process has finishing notifying any other relevant users.
     */
    virtual void Cancel(FMatchmakingEngineRequestHandle Handle) = 0;

    /**
     * Submits the match results to the online stats service, updating each player's _mu and _sigma stats based
     * on which teams won.
     *
     * This function should only be called on the match host (the one running the listen server), not clients.
     *
     * @param InStatsInterface      The IOnlineStats interface from the online subsystem.
     * @param SkillStatPrefix       Should match the skill stat prefix originally provided for the match.
     * @param OriginalMatchResponse Should be the original response that was received for the match.
     * @param TeamScores            For each team in the original match response, the score for that team (indexes are
     *                              aligned). The higher the score, the more that team "won" the match. If you don't
     *                              have scores in your game, just make up some high and low values (like "1" and "0")
     *                              to indicate the winner. The score amount doesn't impact skill updating; it's simply
     *                              used to derive the ranking of teams.
     * @param OnComplete            Called when the stats have been updated.
     */
    virtual void SubmitMatchResults(
        const TSharedRef<const FUniqueNetId> &LocalUserId,
        const TSharedRef<class IOnlineStats, ESPMode::ThreadSafe> &InStatsInterface,
        const FString &SkillStatPrefix,
        const FMatchmakingEngineResponse &OriginalMatchResponse,
        const TArray<double> &TeamScores,
        const FMatchmakingEngineSubmitComplete &OnComplete);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()