// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3356904104, Redpoint::EOS::Presence)
{

/**
 * Represents the online/idle/etc. activity state of a user.
 */
enum class EPresenceUserActivityState : uint8
{
    // The user is offline or is not advertising presence.
    Offline,

    // The user is busy or has turned on do-not-disturb.
    DoNotDisturb,

    // The user is idle and hasn't sent keyboard/mouse/controller input to the game in a while.
    Idle,

    // The user is online and active.
    Online,
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(3356904104, Redpoint::EOS::Presence, EPresenceUserActivityState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()