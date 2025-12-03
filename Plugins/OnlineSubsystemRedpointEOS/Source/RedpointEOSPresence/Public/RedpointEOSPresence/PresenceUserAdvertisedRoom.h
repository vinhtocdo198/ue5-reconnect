// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

// @todo: Figure out why this warning is emitted for the include tree in OnlineSubsystemRedpointEOS.cpp, when FRoomIdRef
// is clearly used.
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()
#include "RedpointEOSRooms/RoomId.h"
REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END()
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3258393336, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSPRESENCE_API FPresenceUserAdvertisedRoom
{
public:
    FPresenceUserAdvertisedRoom(const FRoomIdRef &InRoomId, const FName &InRoomNamespace);

    FRoomIdRef RoomId;
    FName RoomNamespace;
    uint32 CurrentMemberCount;
    uint32 MaximumMemberCount;
    bool bJoinable;
    /**
     * The secret password required to join this room. This value must not be visible in plaintext to anyone who has not
     * joined the room.
     */
    FString SecretPassword;
    bool bRequiresSecretPassword;
    bool bIsMatch;
    bool bMatchStarted;

    REDPOINTEOSPRESENCE_API friend uint32 GetTypeHash(const FPresenceUserAdvertisedRoom &);
};

#if !REDPOINT_EOS_UE_5_4_OR_LATER
REDPOINTEOSPRESENCE_API uint32 GetTypeHash(const TOptional<FPresenceUserAdvertisedRoom> &);
#endif

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(3258393336, Redpoint::EOS::Presence, FPresenceUserAdvertisedRoom)
#if !REDPOINT_EOS_UE_5_4_OR_LATER
REDPOINT_EOS_FILE_NS_EXPORT(3258393336, Redpoint::EOS::Presence, GetTypeHash)
#endif
}

#endif

REDPOINT_EOS_CODE_GUARD_END()