// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/OnlineReplStructs.h"
#include "OnlineBeaconHost.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Containers/Ticker.h"
#include "DedicatedServerMatchmakingBeaconClientMatchMember.h"
#include "DedicatedServerReservation.h"
#include "GameFramework/GameModeBase.h"
#endif

#include "DedicatedServerMatchmakingBeaconHost.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnReservationStatusChanged,
    bool,
    bIsReserved,
    const FDedicatedServerReservation &,
    Reservation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchReadyToStart, const FDedicatedServerReservation &, Reservation);

/**
 * Player controllers clear their NetConnection before the game mode's Logout function is called, so we have to keep
 * track of that mapping in OnPreLogin.
 */
USTRUCT()
struct FTrackedPlayerController
{
    GENERATED_BODY();

public:
    UPROPERTY()
    TObjectPtr<APlayerController> PC = nullptr;

    UPROPERTY()
    FUniqueNetIdRepl UserId;
};

UCLASS(BlueprintType)
class MATCHMAKING_API ADedicatedServerMatchmakingBeaconHost : public AActor
{
    GENERATED_BODY()

    friend class ADedicatedServerMatchmakingBeaconHostObject;
    friend class UMatchmakerSubsystem;
    friend class URedpointMatchmakerTeamComponent;

private:
    UPROPERTY()
    TObjectPtr<AOnlineBeaconHost> Beacon;

#if !REDPOINT_EOS_IS_FREE_EDITION
    /**
     * If true, this dedicated server is reserved for a connecting match.
     */
    bool bIsReserved;

    /**
     * If true, this dedicated server is currently playing a match.
     */
    bool bIsMatchPlaying;

    /**
     * If reserved, this is the user ID that has the reservation (match host). Only the match host can abandon the
     * reservation.
     */
    FUniqueNetIdRepl ReservedByUserId;

    /**
     * When a player reserves a dedicated server, a timeout is set in which at least the match host must connect and
     * stay connected before the reservation is automatically released.
     */
    FTSTicker::FDelegateHandle ReservationTimeoutHandle;

    /**
     * The members in the reservation.
     */
    TArray<FDedicatedServerMatchmakingBeaconClientMatchMember> ReservationMembers;

    /**
     * The sizes of the teams in the reservation, including empty slots that the matchmaker could not fill in a partial
     * match.
     */
    TArray<int32> ReservationTeamSizes;

    /**
     * The members who are yet to connect.
     */
    TSet<FString> RemainingReservationMembers;

    /**
     * The players that are currently connected and their user IDs.
     */
    TArray<FTrackedPlayerController> TrackedConnectedPlayers;

    bool CheckForReservationRelease(float DeltaSeconds);
    void StartMatch();
    bool ReserveForMatch(
        const FUniqueNetIdRepl &ClientUserId,
        const TArray<FDedicatedServerMatchmakingBeaconClientMatchMember> &MatchMembers,
        const TArray<int32> &TeamSizes);
    void OnPreLogin(AGameModeBase *GameMode, const FUniqueNetIdRepl &NewPlayer, FString &ErrorMessage);
    void OnPostLogin(AGameModeBase *GameMode, APlayerController *NewPlayer);
    void OnLogout(AGameModeBase *GameMode, AController *Controller);
    FDedicatedServerReservation CreateReservationInfoForEvent() const;
#endif

public:
    ADedicatedServerMatchmakingBeaconHost();

    /**
     * The port that the beacon will be listening on. This must match the port specified in the matchmaking request for
     * clients.
     */
    UPROPERTY(EditAnywhere, meta = (Category = "Matchmaking"))
    int32 BeaconPort = MatchmakingDedicatedServerDefaultPort;

    /**
     * The timeout in seconds between the reservation being made and when players are expected to have connected. If all
     * players connect before this timeout expires, the match starts immediately regardless of the
     * bRequireAllPlayersConnectedForMatchStart setting.
     *
     * The bRequireAllPlayersConnectedForMatchStart controls what happens if this timeout is reached and not all players
     * have connected.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Matchmaking"))
    float ReservationTimeoutSeconds = 60.0f;

    /**
     * The game match automatically starts when all players are connected. However, if not all players
     * connect within the reservation timeout, this setting controls what happens.
     *
     * If set to true, the reservation is cancelled and all players are kicked unless every player is connected as
     * expected from the matchmaking results.
     *
     * If set to false (the default), the reservation will be kept and the match started as long as the match leader is
     * present. If the match leader isn't present, the reservation is cancelled and all players are kicked.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Matchmaking"))
    bool bRequireAllPlayersConnectedForMatchStart = false;

    /**
     * Allow players to reconnect to the game server as long as they're listed in the current match.
     *
     * If this is turned off, players can not reconnect to the server once the match starts.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Matchmaking"))
    bool bAllowPlayerReconnections = true;

    /**
     * This event is raised when the server is reserved or unreserved. You should implement this event and use it to set
     * a "reserved" attribute on the game session, and then use that "reserved" attribute to filter out dedicated
     * servers from client searches. This will reduce the number of clients that try to reserve already in-use servers.
     */
    UPROPERTY(BlueprintAssignable, meta = (Category = "Matchmaking"))
    FOnReservationStatusChanged OnReservationStatusChanged;

    /**
     * This event is raised when the match is ready to start. You should implement this function and use it to call
     *
     * StartSession on the game session.
     */
    UPROPERTY(BlueprintAssignable, meta = (Category = "Matchmaking"))
    FOnMatchReadyToStart OnMatchReadyToStart;

    /**
     * You should call this function after calling EndSession or DestroySession on the game session. This unreserves the
     * dedicated server and allows it to be reserved again for a new set of players. The OnReservationStatusChanged
     * event will fire as a result of calling this function.
     */
    UFUNCTION(BlueprintCallable, meta = (Category = "Matchmaking"))
    void MatchEnded();

    /**
     * Returns whether or not this dedicated server is currently reserved.
     */
    UFUNCTION(BlueprintPure, meta = (Category = "Matchmaking"))
    bool IsReserved() const;

    /**
     * Returns whether or not this dedicated server is in an active match (true between OnMatchReadyToStart being raised
     * and MatchEnded being called).
     */
    UFUNCTION(BlueprintPure, meta = (Category = "Matchmaking"))
    bool IsMatchPlaying() const;

    /**
     * Returns the user IDs of players who are still yet to connect to the dedicated server, based on the matchmaking
     * reservation. You can use this information on the dedicated server to inform clients (via an RPC or other
     * replicated variable) of who else still needs to connect to the server for the game to start.
     */
    UFUNCTION(BlueprintPure, meta = (Category = "Matchmaking"))
    TSet<FString> GetRemainingReservationMembers() const;

    /**
     * Returns the current matchmaking reservation for this server. The returned "Reservation" only contains valid data
     * if "Is Reserved" is also true. The reservation information is also passed to the "On Reservation Status Changed"
     * and "On Match Ready To Start" events, but you can use this call on subsequent frames to avoid storing the
     * reservation in a separate variable.
     *
     * Along with "Get Remaining Reservation Members", you can use this to calculate all of the members, what teams they
     * are on and whether they still need to connect, and synchronise this information down to connected clients so that
     * players know who they are waiting on in order for the match to start.
     */
    UFUNCTION(BlueprintPure, meta = (Category = "Matchmaking"))
    void GetCurrentReservation(bool &bOutIsReserved, FDedicatedServerReservation &OutReservation) const;

#if !REDPOINT_EOS_IS_FREE_EDITION
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;
#endif
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()