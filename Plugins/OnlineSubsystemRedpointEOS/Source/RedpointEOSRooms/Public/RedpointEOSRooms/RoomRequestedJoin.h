// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomSearchResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3844011396, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API IRoomRequestedJoin
{
public:
    IRoomRequestedJoin() = default;
    UE_NONCOPYABLE(IRoomRequestedJoin);
    virtual ~IRoomRequestedJoin() = default;

    /**
     * Returns the user ID that should perform the JoinRoom operation.
     */
    virtual FRoomUserId GetActorUserId() const = 0;

    /**
     * Returns the search result associated with the request.
     */
    virtual FRoomSearchResultRef GetRoomSearchResult() const = 0;
};

typedef TSharedRef<IRoomRequestedJoin> FRoomRequestedJoinRef;
typedef TSharedPtr<IRoomRequestedJoin> FRoomRequestedJoinPtr;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(3844011396, Redpoint::EOS::Rooms, IRoomRequestedJoin)
REDPOINT_EOS_FILE_NS_EXPORT(3844011396, Redpoint::EOS::Rooms, FRoomRequestedJoinRef)
REDPOINT_EOS_FILE_NS_EXPORT(3844011396, Redpoint::EOS::Rooms, FRoomRequestedJoinPtr)
}

REDPOINT_EOS_CODE_GUARD_END()