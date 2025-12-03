// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceUserActivityState.h"
#include "RedpointEOSPresence/PresenceUserAdvertisedRoom.h"
#include "RedpointEOSRooms/RoomAttribute.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1841536699, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::API::LobbySessionsShared;
using namespace ::Redpoint::EOS::Presence;

/**
 * Represents the whole presence state of a user.
 */
class REDPOINTEOSPRESENCE_API FPresenceUserState
{
public:
    FPresenceUserState(const FIdentityUserId &InUserId);
    FPresenceUserState(const FPresenceUserState &InUserState);
    ~FPresenceUserState() = default;

    FPresenceUserState &operator=(const FPresenceUserState &InUserState);

    /**
     * The user that this presence state is for.
     */
    const FIdentityUserId UserId;

    /**
     * If false, presence will not be advertised. This value is synced based on the runtime platform integration.
     */
    bool bPresenceAdvertised;

    /**
     * If true, the user is currently playing this game. This value is synced based on the runtime platform integration,
     * and is false if the presence is partial from a runtime platform integration, and true if a cross-platform
     * presence lobby actually exists for this user.
     */
    bool bIsPlayingThisGame;

    /**
     * The game session this player is advertising.
     */
    TOptional<FPresenceUserAdvertisedRoom> AdvertisedSession;

    /**
     * The party this player is advertising.
     */
    TOptional<FPresenceUserAdvertisedRoom> AdvertisedParty;

    /**
     * The user's current activity state (online/idle/etc.)
     */
    EPresenceUserActivityState ActivityState;

    /**
     * The text formatting string to be used for the user's status. All FText instances used in the status text must be
     * localised, as some platforms force localisable text (and arbitrary FText/FString will not be displayed on those
     * platforms).
     */
    FTextFormat StatusTextFormat;

    /**
     * The named formatting arguments for the user's status.
     */
    FFormatNamedArguments StatusTextFormatArguments;

    /**
     * Additional custom attributes included in the user's presence information.
     */
    TMap<FString, FRoomAttribute> CustomAttributes;

    /**
     * Return the formatted status text.
     */
    FText GetStatusTextFormatted() const;

    /**
     * Compute a hash of the current presence state for determining whether the current
     * state is out-of-date with the desired state.
     */
    REDPOINTEOSPRESENCE_API friend uint32 GetTypeHash(const FPresenceUserState &);
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(1841536699, Redpoint::EOS::Presence, FPresenceUserState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()