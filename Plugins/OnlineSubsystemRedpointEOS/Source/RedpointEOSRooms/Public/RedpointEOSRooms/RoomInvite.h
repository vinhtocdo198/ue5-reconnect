// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomInviteId.h"
#include "RedpointEOSRooms/RoomSearchResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3636345049, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API IRoomInvite
{
public:
    IRoomInvite() = default;
    UE_NONCOPYABLE(IRoomInvite);
    virtual ~IRoomInvite() = default;

    /**
     * Returns the unique ID of the room invite.
     */
    virtual FRoomInviteId GetRoomInviteId() const = 0;

    /**
     * Returns the user ID that sent the room invite.
     */
    virtual FRoomUserId GetRoomInviteSenderUserId() const = 0;

    /**
     * Returns the user ID that received the room invite.
     */
    virtual FRoomUserId GetRoomInviteRecipientUserId() const = 0;

    /**
     * Returns the "search result" which can be used to join the room via the invite. When you join this search result,
     * the invitation is considered accepted and the sending user may be notified of acceptance.
     */
    virtual TSharedRef<IRoomSearchResult> GetRoomInviteSearchResult() const = 0;
};

typedef TSharedRef<IRoomInvite> FRoomInviteRef;
typedef TSharedPtr<IRoomInvite> FRoomInvitePtr;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(3636345049, Redpoint::EOS::Rooms, IRoomInvite)
REDPOINT_EOS_FILE_NS_EXPORT(3636345049, Redpoint::EOS::Rooms, FRoomInviteRef)
REDPOINT_EOS_FILE_NS_EXPORT(3636345049, Redpoint::EOS::Rooms, FRoomInvitePtr)
}

REDPOINT_EOS_CODE_GUARD_END()