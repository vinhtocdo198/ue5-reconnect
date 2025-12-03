// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/Requests/FeatureRequestsOnRequest.h"
#include "RedpointEOSRooms/RoomAttribute.h"
#include "RedpointEOSRooms/RoomScope.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2655719296, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;

class REDPOINTEOSROOMS_API FCreateRoomRequest : public FFeatureRequestsOnRequest
{
public:
    /**
     * Initialize a create room request with the default number of maximum members (4).
     */
    FCreateRoomRequest(ERoomScope InRoomScope, FRoomUserId InLocalUser, FName InRoomProvider, FName InRoomNamespace);

    /**
     * Initialize a create room request with the specified number of maximum members.
     */
    FCreateRoomRequest(
        ERoomScope InRoomScope,
        FRoomUserId InLocalUser,
        FName InRoomProvider,
        FName InRoomNamespace,
        uint32 InMaxMemberCount);

    /**
     * The local scope of the new room.
     */
    const ERoomScope RoomScope;

    /**
     * The local user who is creating the room.
     */
    const FRoomUserId LocalUser;

    /**
     * The underlying provider of the room; should be either "Lobby" or "Session".
     */
    const FName RoomProvider;

    /**
     * The namespace this room should be created in. Must not be NAME_All.
     */
    const FName RoomNamespace;

    /**
     * The maximum number of members permitted in the room.
     */
    const uint32 MaxMemberCount;

    /**
     * The publicly visible attributes to attach to the room.
     */
    TMap<FString, FRoomAttribute> Attributes;

    /**
     * The publicly visible attributes to attach the owner as a member of the room.
     */
    TMap<FString, FRoomAttribute> MemberAttributes;
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(2655719296, Redpoint::EOS::Rooms::Requests, FCreateRoomRequest)
}

REDPOINT_EOS_CODE_GUARD_END()