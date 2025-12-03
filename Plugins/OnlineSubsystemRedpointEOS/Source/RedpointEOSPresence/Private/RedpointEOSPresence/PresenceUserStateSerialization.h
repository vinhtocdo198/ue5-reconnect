// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSRooms/Requests/UpdateRoomRequest.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomSearchResult.h"
#include "RedpointEOSRooms/RoomSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1199869237, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Identity;

/**
 * Handles serializing and deserializing presence information.
 */
class FPresenceUserStateSerialization
{
private:
    static void SerializeForCreate(
        FCreateRoomRequest &CurrentRequest,
        bool bParty,
        const TOptional<FPresenceUserAdvertisedRoom> &DesiredLocalUserAdvertisedRoom);
    static void SerializeForUpdate(
        FUpdateRoomRequest &CurrentRequest,
        const FRoomRef &CurrentPresenceRoom,
        bool bParty,
        const TOptional<FPresenceUserAdvertisedRoom> &DesiredLocalUserAdvertisedRoom);
    static TOptional<FPresenceUserAdvertisedRoom> Deserialize(
        const FIdentityUserId &TargetAccountId,
        const FRoomSearchResultRef &RoomSearchResult,
        bool bParty);

public:
    static void SerializeForCreate(
        FCreateRoomRequest &Request,
        const FPresenceUserState &DesiredLocalUserStateSnapshot);
    static void SerializeForUpdate(
        FUpdateRoomRequest &Request,
        const FRoomRef &CurrentPresenceRoom,
        const FPresenceUserState &DesiredLocalUserStateSnapshot);
    static FPresenceUserState Deserialize(
        const FIdentityUserId &TargetAccountId,
        const FRoomSearchResultRef &RoomSearchResult);
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(1199869237, Redpoint::EOS::Presence, FPresenceUserStateSerialization)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()