// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3564402838, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

enum class ERoomJoinReason : uint8
{
    /** The user is joining the room because CreateRoom was called on the room system. */
    RequestedByCreateRoom,

    /** The user is joining the room because JoinRoom was called on the room system. */
    RequestedByJoinRoom,

    /** The user is joining the room because they clicked "Join Game" from an overlay. */
    JoinedViaOverlay,

    /** The user is joining the room because the accepted an incoming invite. */
    AcceptedInvite,
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(3564402838, Redpoint::EOS::Rooms, ERoomJoinReason)
}

REDPOINT_EOS_CODE_GUARD_END()