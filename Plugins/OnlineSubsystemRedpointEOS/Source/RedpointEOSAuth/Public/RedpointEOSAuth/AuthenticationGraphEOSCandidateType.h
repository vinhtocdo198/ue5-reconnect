// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1393263648, Redpoint::EOS::Auth)
{

/**
 * The type of EOS candidate. This is used by authentication graph nodes to select particular candidates.
 */
enum class EAuthenticationGraphEOSCandidateType
{
    /** This is a generic EOS candidate. */
    Generic,

    /** This is a cross-platform candidate, such as Epic Games or another first party provider. */
    CrossPlatform,

    /** This is a device ID candidate. */
    DeviceId,
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(1393263648, Redpoint::EOS::Auth, EAuthenticationGraphEOSCandidateType)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()