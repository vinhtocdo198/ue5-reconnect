// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(712022924, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

/**
 * The reason why this slot is changing state.
 */
enum EIdentityUserSlotChangeReason
{
    // The reason is not known.
    Unknown,

    // The Login function was explicitly called by the game and the user completed sign in.
    ExplicitLogin,

    // The Logout function was explicitly called by the game and the user signed out.
    ExplicitLogout,

    // The Login function was explicitly called by the game with an already signed in account, and the user went through
    // the linking process.
    ExplicitLink,
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(712022924, Redpoint::EOS::Identity, EIdentityUserSlotChangeReason)
}

REDPOINT_EOS_CODE_GUARD_END()