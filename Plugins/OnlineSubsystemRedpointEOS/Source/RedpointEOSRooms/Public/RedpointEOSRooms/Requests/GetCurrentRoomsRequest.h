// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomAttributeConstraint.h"
#include "RedpointEOSRooms/RoomScope.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1758651349, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FGetCurrentRoomsRequest
{
public:
    /**
     * Initialize a get rooms request, for getting globally-scoped rooms.
     */
    FGetCurrentRoomsRequest(
        ERoomScope InRoomScope,
        FName InRoomNamespace,
        const TMap<FString, FRoomAttributeConstraint> &InAttributeConstraints =
            TMap<FString, FRoomAttributeConstraint>());

    /**
     * Initialize a get rooms request, for getting rooms potentially scoped to a user.
     */
    FGetCurrentRoomsRequest(
        ERoomScope InRoomScope,
        FRoomUserId InLocalUserId,
        FName InRoomNamespace,
        const TMap<FString, FRoomAttributeConstraint> &InAttributeConstraints =
            TMap<FString, FRoomAttributeConstraint>());

    /**
     * The scope of rooms to retrieve.
     */
    const ERoomScope RoomScope;

    /**
     * The user to fetch rooms for, if the scope is ERoomScope::User.
     */
    const FRoomUserId LocalUserId;

    /**
     * The namespace to return rooms for. Can be NAME_All to return current rooms across all namespaces.
     */
    const FName RoomNamespace;

    /**
     * If at least one constraint is set, only locally joined rooms that match the attribute constraints will be
     * returned.
     */
    const TMap<FString, FRoomAttributeConstraint> AttributeConstraints;
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(1758651349, Redpoint::EOS::Rooms::Requests, FGetCurrentRoomsRequest)
}

REDPOINT_EOS_CODE_GUARD_END()