// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/MatchmakingErrors.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define ONLINE_ERROR_NAMESPACE "errors.com.redpoint.matchmaking"

const FOnlineError &OnlineMatchmaking::Errors::MissingIdentityImplementation()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.missing_identity_implementation"),
        NSLOCTEXT("Matchmaking", "Error_MissingIdentityImplementation", "Request is missing identity implementation"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::MissingLobbyImplementation()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.missing_lobby_implementation"),
        NSLOCTEXT("Matchmaking", "Error_MissingLobbyImplementation", "Request is missing lobby implementation"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::MissingStatsImplementation()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.missing_stats_implementation"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_MissingStatsImplementation",
            "Request is missing stats implementation, which is required for skill-based matchmaking"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::MissingUserId()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.missing_user_id"),
        NSLOCTEXT("Matchmaking", "Error_MissingUserId", "Request is missing user ID"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::MissingRequiredInterfaces()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.missing_required_interfaces"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_MissingRequiredInterfaces",
            "Online subsystem does not provide required interfaces for matchmaking to operate"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::AtLeastOneTeamRequired()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.at_least_one_team_required"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_AtLeastOneTeamRequired",
            "You must have at least one team defined for matchmaking"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::MissingPartyImplementationOnClient()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.missing_party_implementation_on_client"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_MissingPartyImplementationOnClient",
            "Missing host configuration, and can't operate as a client without a party system"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::MissingPartyIdOnClient()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.missing_party_id_on_client"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_MissingPartyIdOnClient",
            "Missing host configuration, and can't operate as a client without a party ID"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::FailedToLoadEssentialSkillStats()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.failed_to_load_essential_skill_stats"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_FailedToLoadEssentialSkillStats",
            "Skill-based matchmaking was enabled, but we couldn't load the skill stats for the host or party members"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::PartyTooLargeForFirstTeam()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.party_too_large_for_first_team"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_PartyTooLargeForFirstTeam",
            "The party was too large to assign to the first team in the match"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::ClientLeftParty()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.client_left_party"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_ClientLeftParty",
            "Client left the party that they were in for matchmaking, thus there will be no party host to "
            "proceed with matchmaking"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::ClientUnableToJoinHostLobby()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.client_unable_to_join_host_lobby"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_ClientUnableToJoinHostLobby",
            "Client was unable to join the party leader's lobby as they entered matchmaking"));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::ClientNotMemberOfParty()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.client_not_member_of_party"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_ClientNotMemberOfParty",
            "Client entered matchmaking in a party but was not actually a member of that party according to "
            "the online subsystem."));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::RequestUserIsNotLocallySignedIn()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.request_user_is_not_locally_sign_in"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_RequestUserIsNotLocallySignedIn",
            "The user that entered matchmaking is not signed into the local subsystem."));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::GlobalEngineNotAvailable()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.global_engine_not_available"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_GlobalEngineNotAvailable",
            "The GEngine value is null, and it must be present in order to change maps."));
    return Err;
}

FOnlineError OnlineMatchmaking::Errors::FailedToBrowseToMap(const FString &InEngineError)
{
    return ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.failed_to_browse_to_map"),
        FText::Format(
            NSLOCTEXT(
                "Matchmaking",
                "Error_FailedToBrowseToMap",
                "Unable to browse to the specified map or URL. The engine error was: {0}"),
            FText::FromString(InEngineError)));
}

const FOnlineError &OnlineMatchmaking::Errors::StartedMapDoesNotHaveNetDriver()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.started_map_does_not_have_net_driver"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_StartedMapDoesNotHaveNetDriver",
            "The started map does not have a NetDriver set."));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::NetDriverInWrongMode()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.net_driver_in_wrong_mode"),
        NSLOCTEXT("Matchmaking", "Error_NetDriverInWrongMode", "The NetDriver of the map is in the wrong mode."));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::UnableToCreateGameSession()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.unable_to_create_game_session"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_UnableToCreateGameSession",
            "Unable to create the game session on the server."));
    return Err;
}

const FOnlineError &OnlineMatchmaking::Errors::UnableToUpdateLobbyWithConnectionString()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.unable_to_update_lobby_with_connection_string"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_UnableToUpdateLobbyWithConnectionString",
            "Unable to update the matchmaking lobby with the game server's connection string."));
    return Err;
}

FOnlineError OnlineMatchmaking::Errors::UnableToUpdateLobbyWithConnectionString(const FOnlineError &InLobbyError)
{
    return ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.unable_to_update_lobby_with_connection_string"),
        FText::Format(
            NSLOCTEXT(
                "Matchmaking",
                "Error_UnableToUpdateLobbyWithConnectionString_WithError",
                "Unable to update the matchmaking lobby with the game server's connection string. The lobby API error "
                "was: {0}"),
            FText::FromString(InLobbyError.ToLogString())));
}

const FOnlineError &OnlineMatchmaking::Errors::IncorrectNumberOfTeamScores()
{
    static FOnlineError Err = ONLINE_ERROR(
        EOnlineErrorResult::FailExtended,
        TEXT("errors.com.redpoint.matchmaking.incorrect_number_of_team_scores"),
        NSLOCTEXT(
            "Matchmaking",
            "Error_IncorrectNumberOfTeamScores",
            "The number of team scores must match the number of teams when submitting match results"));
    return Err;
}

#undef ONLINE_ERROR_NAMESPACE

#endif

REDPOINT_EOS_CODE_GUARD_END()