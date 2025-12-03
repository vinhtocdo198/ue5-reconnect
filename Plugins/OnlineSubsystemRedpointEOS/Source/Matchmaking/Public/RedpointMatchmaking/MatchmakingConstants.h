// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

constexpr auto MatchmakingFollowRequestLobbyCapacity = 64;

constexpr auto MatchmakingMetadataLobbyType = TEXT("type");
constexpr auto MatchmakingMetadataLobbyStatus = TEXT("status");
constexpr auto MatchmakingMetadataLobbyRequestId = TEXT("request_id");
constexpr auto MatchmakingMetadataLobbyEtaUtcUnixTimestamp = TEXT("eta_utc_unix_timestamp");
constexpr auto MatchmakingMetadataLobbyHeartbeatTimestamp = TEXT("heartbeat");
constexpr auto MatchmakingMetadataLobbySkillOrdinal = TEXT("mmr_ordinal");
constexpr auto MatchmakingMetadataQueueName = TEXT("queue");
constexpr auto MatchmakingMetadataFollowRequestLobbyId = TEXT("follow_request_lobby_id");
constexpr auto MatchmakingMetadataFollowRequestLobbySecret = TEXT("follow_request_lobby_secret");
constexpr auto MatchmakingMetadataFollowTargetLobbyId = TEXT("follow_target_lobby_id");
constexpr auto MatchmakingMetadataFollowTargetLobbySecret = TEXT("follow_target_lobby_secret");
constexpr auto MatchmakingMetadataResult = TEXT("result");
constexpr auto MatchmakingMetadataConnectionSessionId = TEXT("connection_session_id");
constexpr auto MatchmakingMetadataConnectionUrl = TEXT("connection_url");
constexpr auto MatchmakingMetadataTargetCapacity = TEXT("target_capacity");

constexpr auto MatchmakingAbandonKey = TEXT("abandon");
constexpr auto MatchmakingAbandonPartyRestart = TEXT("restart");
constexpr auto MatchmakingAbandonPartyCancel = TEXT("cancel");
constexpr auto MatchmakingAbandonMatch = TEXT("abandon");

constexpr auto MatchmakingMemberMetadataFollowRequest = TEXT("follow_request");
constexpr auto MatchmakingMemberMetadataFollowResponse = TEXT("follow_response");

constexpr auto MatchmakingFollowResponseAccepted = TEXT("Accepted");
constexpr auto MatchmakingFollowResponseRejected = TEXT("Rejected");

constexpr auto MatchmakingMemberMetadataVirtualizedChatMessage = TEXT("virtualized_chat_message");
constexpr auto MatchmakingMemberMetadataTeam = TEXT("team");
constexpr auto MatchmakingMemberMetadataFinalized = TEXT("finalized");
constexpr auto MatchmakingMemberMetadataConnecting = TEXT("connecting");

constexpr auto MatchmakingLobbyTypeParty = TEXT("party");
constexpr auto MatchmakingLobbyTypeMatch = TEXT("match");
constexpr auto MatchmakingLobbyTypeMatchFollowRequests = TEXT("match_follow_requests");
constexpr auto MatchmakingLobbyTypeGame = TEXT("game");

constexpr auto MatchmakingStatusIniting = TEXT("initing");
constexpr auto MatchmakingStatusFilling = TEXT("filling");
constexpr auto MatchmakingStatusSearching = TEXT("searching");
constexpr auto MatchmakingStatusWaiting = TEXT("waiting");
constexpr auto MatchmakingStatusFollowing = TEXT("following");
constexpr auto MatchmakingStatusFinalizing = TEXT("finalizing");

constexpr auto PartyMetadataMatchmakingLobbyId = TEXT("match_lobby_id");
constexpr auto PartyMetadataLobbyStatus = TEXT("status");

constexpr auto PartyStatusQueued = TEXT("queued");

/**
 * The amount of time in seconds a host will wait for a potential candidate to respond to it's follow request.
 */
constexpr auto MatchmakingTimingFollowRequestTimeout = 8.0f;
/**
 * The amount of time in seconds a host will wait for additional follow requests to arrive for consideration before
 * it chooses a request to respond to.
 */
constexpr auto MatchmakingTimingIncomingRequestQueueDelay = 3.0f;
/**
 * The amount of time in seconds a client will wait before it determines that a host is no longer responding.
 */
constexpr auto MatchmakingTimingHeartbeatTimeout = 60.0f;
/**
 * The frequency in seconds that a host will update it's heartbeat attribute in it's lobby.
 */
constexpr auto MatchmakingTimingHeartbeatFrequency = 15.0f;
/**
 * The lower bound of the baseline variance in searching delay.
 */
constexpr auto MatchmakingTimingSearchingVarianceLowerDelay = 1.0f;
/**
 * The upper bound of the baseline variance in searching delay.
 */
constexpr auto MatchmakingTimingSearchingVarianceUpperDelay = 2.5f;
/**
 * The lower bound of the variance added per rejection to the search delay.
 */
constexpr auto MatchmakingTimingSearchingPerRejectionLowerDelay = 3.0f;
/**
 * The upper bound of the variance added per rejection to the search delay.
 */
constexpr auto MatchmakingTimingSearchingPerRejectionUpperDelay = 9.0f;
/**
 * The minimum time in seconds between search operations.
 */
constexpr auto MatchmakingTimingSearchingMinimumDelay = 1.0f;
/**
 * The amount of time in seconds to wait for players to join after the follow request is accepted.
 */
constexpr auto MatchmakingTimingWaitForPlayersTimeout = 30.0f;

#endif

/**
 * The default port that the matchmaking dedicated server beacon listens on. Instead of modifying the constant here, you
 * can change port on the beacon host and in the matchmaking configuration.
 */
constexpr auto MatchmakingDedicatedServerDefaultPort = 9990;

REDPOINT_EOS_CODE_GUARD_END()