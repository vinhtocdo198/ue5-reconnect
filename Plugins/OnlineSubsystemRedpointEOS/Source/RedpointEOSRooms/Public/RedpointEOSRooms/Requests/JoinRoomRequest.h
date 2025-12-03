// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/Requests/FeatureRequestsOnRequest.h"
#include "RedpointEOSRooms/RoomInvite.h"
#include "RedpointEOSRooms/RoomRequestedJoin.h"
#include "RedpointEOSRooms/RoomScope.h"
#include "RedpointEOSRooms/RoomSearchResult.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3330400371, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;

class REDPOINTEOSROOMS_API FJoinRoomRequest : public FFeatureRequestsOnRequest
{
public:
    /**
     * Initialize a join room request, joining from the specified search result.
     */
    FJoinRoomRequest(ERoomScope InRoomScope, FRoomUserId InLocalUser, const FRoomSearchResultRef &InSearchResult);

    /**
     * Initialize a join room request, joining by the specified room ID.
     */
    FJoinRoomRequest(ERoomScope InRoomScope, FRoomUserId InLocalUser, const FRoomIdRef &InRoomId);

    /**
     * Initialize a join room request, joining by the specified room invite. You should use this version if you have
     * an instance of FRoomInvite, as it will ensure the room invite will be removed once used.
     */
    FJoinRoomRequest(ERoomScope InRoomScope, const FRoomInviteRef &InRoomInvite);

    /**
     * Initialize a join room request, joining by the requested join operation. You should use this version if you have
     * an instance of FRoomRequestedJoinRef, as it will ensure the room invite will be removed once used.
     */
    FJoinRoomRequest(ERoomScope InRoomScope, const FRoomRequestedJoinRef &InRoomRequestedJoin);

    /**
     * The local scope of the new room.
     */
    const ERoomScope RoomScope;

    /**
     * The local user who is joining the room.
     */
    const FRoomUserId LocalUser;

    /**
     * If set, this join request uses the specified search result.
     */
    const FRoomSearchResultPtr SearchResult;

    /**
     * If set, this join request uses the specified room ID.
     */
    const FRoomIdPtr RoomId;

    /**
     * If set, this join request uses the specified room invite.
     */
    const FRoomInvitePtr RoomInvite;

    /**
     * If set, this join request uses the specified requested join operation.
     */
    const FRoomRequestedJoinPtr RoomRequestedJoin;

    /**
     * The publicly visible attributes to attach the member joining the room.
     */
    TMap<FString, FRoomAttribute> MemberAttributes;

    /**
     * Returns the room ID resolved from the type of join room request.
     */
    FRoomIdRef GetRoomId() const;

    /**
     * Returns the room invite if there is one associated with the join room request.
     */
    FRoomInvitePtr GetRoomInvite() const;
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(3330400371, Redpoint::EOS::Rooms::Requests, FJoinRoomRequest)
}

REDPOINT_EOS_CODE_GUARD_END()