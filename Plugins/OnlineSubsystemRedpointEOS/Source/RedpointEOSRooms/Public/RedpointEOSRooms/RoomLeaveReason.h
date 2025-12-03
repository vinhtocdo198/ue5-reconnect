// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(268474049, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

enum class ERoomLeaveReason : uint8
{
    /** The user is leaving the room because they left via an overlay (such as "Leave Party") or because LeaveRoom was
     * called. */
    LeftExplicitly,

    /** The member was kicked from the room. */
    KickedFromRoom,

    /** The room was destroyed for all members. */
    RoomDestroyed,

    /** The user lost connection to the lobby or server. */
    Disconnected,
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(268474049, Redpoint::EOS::Rooms, ERoomLeaveReason)
}

REDPOINT_EOS_CODE_GUARD_END()