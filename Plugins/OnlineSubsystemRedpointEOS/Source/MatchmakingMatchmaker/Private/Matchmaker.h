// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "LogMatchmakingMatchmaker.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "RedpointMatchmaking/Utility/UserIdContainers.h"

#include "Matchmaker.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UENUM(BlueprintType)
enum class EMatchmakerHostConfigurationAttributeFilterType : uint8
{
    String,
    Int64,
    Float,
    Boolean,
};

UENUM(BlueprintType)
enum class EMatchmakerHostConfigurationAttributeFilterComparison : uint8
{
    Equal,
    NotEqual,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,
};

USTRUCT(BlueprintType)
struct MATCHMAKINGMATCHMAKER_API FMatchmakerHostConfigurationAttributeFilter
{
    GENERATED_BODY()

public:
    /**
     * The attribute to filter on.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString AttributeName;

    /**
     * The attribute type.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    EMatchmakerHostConfigurationAttributeFilterType AttributeType =
        EMatchmakerHostConfigurationAttributeFilterType::String;

    /**
     * The attribute value, if type is set to string.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString AttributeValueString;

    /**
     * The attribute value, if type is set to int64.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    int64 AttributeValueInt64 = 0;

    /**
     * The attribute value, if type is set to float.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    float AttributeValueFloat = 0.0f;

    /**
     * The attribute value, if type is set to boolean.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    bool AttributeValueBoolean = false;

    /**
     * The comparison type to use.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    EMatchmakerHostConfigurationAttributeFilterComparison Comparison =
        EMatchmakerHostConfigurationAttributeFilterComparison::Equal;
};

USTRUCT(BlueprintType)
struct MATCHMAKINGMATCHMAKER_API FMatchmakerHostConfiguration
{
    GENERATED_BODY()

public:
    FMatchmakerHostConfiguration()
        : QueueName()
        , TeamCapacities()
        , SkillStatPrefix()
        , Map()
        , SessionFilters() {};

    /**
     * The name of the matchmaking queue to queue into.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString QueueName;

    /**
     * A string like '4v4', '3v3v3' or '4x20' (20 teams of 4).
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString TeamCapacities;

    /**
     * If true, finish matchmaking with a partially filled match if no further candidates could be found within the
     * specified timeout.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    bool bAllowPartiallyFilledMatches = true;

    /**
     * If partial matches is enabled, this is the minimum number of players that must be present on each team in order
     * for a partial match to be accepted. If a match does not yet have at least this number of players on each team, it
     * will continue to attempt matchmaking even beyond the configured matchmaking timeout.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches"))
    int32 MinimumTeamMemberCountForPartialFill = 0;

    /**
     * If partial matches is enabled, this is the timeout in seconds after which, if we still can't find any other
     * candidates to match with, a partially filled match will be returned. Note that this is *not* a global timeout for
     * the entire matchmaking process. It is just the time since the last player was added to the match.
     *
     * The matchmaker has a minimum of 15 seconds for this value; if you set this to any lower than 15 seconds, the
     * matchmaker will use 15 seconds instead.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches", ClampMin = "15"))
    int32 NoCandidatesTimeout = 60;

    /**
     * The time in seconds to add to the timeout for each currently empty slot in the match. If this is non-zero,
     * matchmaking will wait longer to return a partial match, the more empty the match currently is.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches", ClampMin = "0"))
    int32 NoCandidatesTimeoutPerEmptySlot = 5;

    /**
     * If true, matchmaking will prioritize having balanced teams throughout the matchmaking process, rather than
     * prioritizing having completely full teams. Enabling this option means a partially filled match is more likely to
     * have a balanced number of players, at the cost of having a higher chance that a match will be partially filled if
     * there are not a lot of solo players entering the matchmaking queue.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches"))
    bool bPrioritizeBalance = false;

    /**
     * If set to something other than empty string, enables skill-based matchmaking. The <prefix>_mu and <prefix>_sigma
     * stats will be looked up in the online stats interface to perform skill-based matchmaking, so you must ensure
     * you've created the appropriate stats in the backend.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString SkillStatPrefix;

    /**
     * If true, the host will search for a dedicated server to play on once the match is ready, instead of hosting the
     * game themselves on a listen server.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    bool bUseDedicatedServers = false;

    /**
     * The map to start when matchmaking is complete (listen servers only).
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Listen Servers)",
        meta = (EditCondition = "!bUseDedicatedServers"))
    TSoftObjectPtr<UWorld> Map;

    /**
     * The filters to apply when searching for dedicated servers.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Dedicated Servers)",
        meta = (EditCondition = "bUseDedicatedServers"))
    TArray<FMatchmakerHostConfigurationAttributeFilter> SessionFilters;

    /**
     * The port that the dedicated server matchmaking beacon listens on.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Dedicated Servers)",
        meta = (EditCondition = "bUseDedicatedServers"))
    FName BeaconPort = FName(TEXT("9990"));
};

USTRUCT(BlueprintType)
struct MATCHMAKINGMATCHMAKER_API FMatchmakerResultTeam
{
    GENERATED_BODY()

public:
    FMatchmakerResultTeam()
        : Slots() {};

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    TMap<int32, FUniqueNetIdRepl> Slots;
};

USTRUCT(BlueprintType)
struct MATCHMAKINGMATCHMAKER_API FMatchmakerResult
{
    GENERATED_BODY()

public:
    FMatchmakerResult()
        : Teams() {};

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    TArray<FMatchmakerResultTeam> Teams;
};

USTRUCT(BlueprintType)
struct MATCHMAKINGMATCHMAKER_API FMatchmakerProgressInfo
{
    GENERATED_BODY()

public:
    FMatchmakerProgressInfo()
        : StepName()
        , CurrentStatus()
        , CurrentDetail()
        , CurrentProgress()
        , EstimatedTimeOfCompletion() {};

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    FName StepName;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    FText CurrentStatus;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    FText CurrentDetail;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    float CurrentProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Matchmaking")
    FDateTime EstimatedTimeOfCompletion = FDateTime();
};

UCLASS(BlueprintType)
class MATCHMAKINGMATCHMAKER_API AMatchmaker : public AActor
{
    GENERATED_BODY()

private:
#if !REDPOINT_EOS_IS_FREE_EDITION
    TWeakPtr<IOnlineIdentity, ESPMode::ThreadSafe> Identity;
    TWeakPtr<IOnlineLobby, ESPMode::ThreadSafe> Lobby;
    TWeakPtr<IOnlinePartySystem, ESPMode::ThreadSafe> PartySystem;
    TWeakPtr<IOnlineSession, ESPMode::ThreadSafe> Session;
    TWeakPtr<IOnlineStats, ESPMode::ThreadSafe> Stats;
    bool bConnectViaSessionID;
    bool bIsUsable;
    TUserIdMap<bool> bLocallyForcedNotReady;
    TSharedPtr<const FUniqueNetId> LastLeader;
    TUserIdMap<FMatchmakingEngineRequestHandle> RequestsByLocalUser;
    TSharedPtr<IMatchmakingEngine> MatchmakingEngine;
    FDelegateHandle PartyJoinedDelegateHandle;
    FDelegateHandle PartyExitedDelegateHandle;
    FDelegateHandle PartyMemberExitedDelegateHandle;
    FDelegateHandle PartyMemberJoinedDelegateHandle;
    FDelegateHandle PartyMemberPromotedDelegateHandle;
    FDelegateHandle PartyMemberDataReceivedDelegateHandle;
    TUserIdMap<bool> bIsMatchmaking;
    TUserIdMap<FMatchmakerProgressInfo> LastMatchmakingProgressData;

    TSharedPtr<const FUniqueNetId> ResolveOptionalUserId(const FUniqueNetIdRepl &UserId, const FString &FunctionName);

    bool IsPartyPrimaryParty(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId);
    void SetMemberReadinessStatus(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId, bool bIsReady);

    void OnNativePartyJoined(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId);
    void OnNativePartyExited(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId);
    void OnNativePartyMemberExited(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const EMemberExitedReason Reason);
    void OnNativePartyMemberJoined(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId);
    void OnNativePartyMemberPromoted(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &NewLeaderId);
    void OnNativePartyMemberDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const FName &Namespace,
        const FOnlinePartyData &PartyMemberData);

    void StartClientMatchmaking(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId);
    void StopClientMatchmaking(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId);
    void StartHostMatchmaking(const FUniqueNetId &LocalUserId, const TSharedPtr<const FOnlinePartyId> &PartyId);
    bool StopHostMatchmaking(const FUniqueNetId &LocalUserId);

    void EvaluatePartyReadiness(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId);

    void OnNativeMatchmakingProgress(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineProgressInfo ProgressInfo);
    void OnNativeMatchmakingCancelled(const FString &TaskId, const struct FMatchmakingEngineRequest &Request);
    void OnNativeMatchmakingError(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        const FOnlineError &Error);
    FSearchParams OnNativeMatchmakingGetDedicatedServerSearchParams(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineResponse Response,
        int32 SearchIteration);

    FName GetMapName();
#endif

public:
    AMatchmaker();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
    void OnProgress(const FMatchmakerProgressInfo &ProgressInfo);

    UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
    void OnCancelled();

    UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
    void OnError(const FString &MatchmakingError);

    /**
     * The matchmaker host configuration. This value is only used by the party leader.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FMatchmakerHostConfiguration HostConfiguration;

    /**
     * Sets the ready state of the current player. When all players are ready in the party, the party leader will queue
     * the party into matchmaking. If your game or online platform doesn't support parties, you still need to call this
     * for solo players.
     *
     * If you don't provide a value for UserId, this function assumes the user ID of Player Controller 0. You only need
     * to provide UserId if you support multiple players (i.e. split screen) queuing into the same party on the same
     * device.
     */
    UFUNCTION(BlueprintCallable, Category = "Matchmaking")
    void SetReadyState(const FUniqueNetIdRepl &UserId, bool bIsReady);

    /**
     * Gets the ready state for the specified player. This works for other party members, so you can indicate ready
     * status per player.
     *
     * If you don't provide a value for LocalUserId, this function assumes the user ID of Player Controller 0.
     * If you don't provide a value for TargetUserId, this function assumes the user ID of Player Controller 0 if
     * LocalUserId was also not provided.
     */
    UFUNCTION(BlueprintPure, Category = "Matchmaking")
    void GetReadyState(const FUniqueNetIdRepl &LocalUserId, const FUniqueNetIdRepl &TargetUserId, bool &bIsReady);

    /**
     * Returns whether *any* local user is undergoing matchmaking. This is a utility function so you don't need to track
     * this via OnProgress/OnCancelled/OnError events if you just want to know if matchmaking is occurring.
     */
    UFUNCTION(BlueprintPure, Category = "Matchmaking")
    void IsMatchmaking(bool &bIsMatchmaking);

    /**
     * Returns the latest matchmaking status reported by the OnProgress event. If you'd prefer to get the matchmaking
     * status directly instead of listening for OnProgress, you can use this function.
     *
     * If matchmaking isn't running, StepName will be NAME_None and the other values are undefined.
     */
    UFUNCTION(BlueprintPure, Category = "Matchmaking")
    void GetMatchmakingStatus(const FUniqueNetIdRepl &UserIdRepl, FMatchmakerProgressInfo &ProgressInfo);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()