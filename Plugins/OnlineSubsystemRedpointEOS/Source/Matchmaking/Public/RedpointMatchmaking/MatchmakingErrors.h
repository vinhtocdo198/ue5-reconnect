// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Online/CoreOnline.h"
#include "OnlineError.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

namespace OnlineMatchmaking
{
namespace Errors
{
MATCHMAKING_API const FOnlineError &MissingIdentityImplementation();
MATCHMAKING_API const FOnlineError &MissingLobbyImplementation();
MATCHMAKING_API const FOnlineError &MissingStatsImplementation();
MATCHMAKING_API const FOnlineError &MissingUserId();
MATCHMAKING_API const FOnlineError &MissingRequiredInterfaces();
MATCHMAKING_API const FOnlineError &AtLeastOneTeamRequired();
MATCHMAKING_API const FOnlineError &MissingPartyImplementationOnClient();
MATCHMAKING_API const FOnlineError &MissingPartyIdOnClient();
MATCHMAKING_API const FOnlineError &FailedToLoadEssentialSkillStats();
MATCHMAKING_API const FOnlineError &PartyTooLargeForFirstTeam();

MATCHMAKING_API const FOnlineError &ClientLeftParty();
MATCHMAKING_API const FOnlineError &ClientUnableToJoinHostLobby();
MATCHMAKING_API const FOnlineError &ClientNotMemberOfParty();

MATCHMAKING_API const FOnlineError &RequestUserIsNotLocallySignedIn();
MATCHMAKING_API const FOnlineError &GlobalEngineNotAvailable();
MATCHMAKING_API FOnlineError FailedToBrowseToMap(const FString &InEngineError);
MATCHMAKING_API const FOnlineError &StartedMapDoesNotHaveNetDriver();
MATCHMAKING_API const FOnlineError &NetDriverInWrongMode();
MATCHMAKING_API const FOnlineError &UnableToCreateGameSession();
MATCHMAKING_API const FOnlineError &UnableToUpdateLobbyWithConnectionString();
MATCHMAKING_API FOnlineError UnableToUpdateLobbyWithConnectionString(const FOnlineError &InLobbyError);

MATCHMAKING_API const FOnlineError &IncorrectNumberOfTeamScores();
}; // namespace Errors
}; // namespace OnlineMatchmaking

#endif

REDPOINT_EOS_CODE_GUARD_END()