// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3877430285, Redpoint::EOS::Rooms)
{

enum class ERoomScope : uint8
{
    /** The room state is shared between all local users in the game. */
    Global,

    /** The room state is stored per local user. */
    User,
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(3877430285, Redpoint::EOS::Rooms, ERoomScope)
}

REDPOINT_EOS_CODE_GUARD_END()