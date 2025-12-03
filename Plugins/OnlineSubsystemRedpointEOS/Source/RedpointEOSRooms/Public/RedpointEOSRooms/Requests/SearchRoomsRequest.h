// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSRooms/RoomAttributeConstraint.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(86934985, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FSearchRoomsRequest
{
public:
    /**
     * Initializes a room search that searches for a room with a specific ID.
     */
    FSearchRoomsRequest(
        const FRoomUserId &InLocalUser,
        const FName &InRoomProvider,
        const FName &InRoomNamespace,
        const FRoomIdRef &InRequireRoomId);

    /**
     * Initializes a room search that searches for rooms with a specific member in them.
     */
    FSearchRoomsRequest(
        const FRoomUserId &InLocalUser,
        const FName &InRoomProvider,
        const FName &InRoomNamespace,
        const FRoomUserId &InRequireRoomMemberId);

    /**
     * Initializes a room search that searches for rooms based on attribute constraints.
     */
    FSearchRoomsRequest(
        const FRoomUserId &InLocalUser,
        const FName &InRoomProvider,
        const FName &InRoomNamespace,
        const TMap<FString, FRoomAttributeConstraint> &InAttributeConstraints);

    /**
     * The local user who is performing the room search.
     */
    const FRoomUserId LocalUser;

    /**
     * The underlying provider of the room; should be either "Lobby" or "Session".
     */
    const FName RoomProvider;

    /**
     * The namespace to search rooms under. Must not be NAME_All.
     */
    const FName RoomNamespace;

    /**
     * The maximum number of results that will be returned from the search.
     */
    TOptional<uint32> MaxResultCount;

    /**
     * If set, searches for the specific room identified by this room ID.
     */
    const FRoomIdPtr RequireRoomId;

    /**
     * If set, searches for the room that contains this member.
     */
    const TOptional<FRoomUserId> RequireRoomMemberId;

    /**
     * The attribute constraints that a room must meet in order to be returned in the search results.
     */
    const TMap<FString, FRoomAttributeConstraint> AttributeConstraints;

    /**
     * If true (the default), the search will be automatically retried if there are no initial search results. This
     * helps mitigate eventual consistency issues. A total of 10 search attempts (including retries) will be made.
     */
    bool bDurable;
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(86934985, Redpoint::EOS::Rooms::Requests, FSearchRoomsRequest)
}

REDPOINT_EOS_CODE_GUARD_END()