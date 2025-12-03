// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(960893349, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Identity;

/**
 * Represents a change in whether we are interested in a target user's presence information.
 */
class REDPOINTEOSPRESENCE_API FPresenceInterestChange
{
public:
    FPresenceInterestChange(const FIdentityUserId &InTargetUserId, bool bInInterested);
    FPresenceInterestChange(const FPresenceInterestChange &InInterestChange);
    ~FPresenceInterestChange() = default;

    /**
     * The remote user ID that we are changing our interest in their presence status.
     */
    const FIdentityUserId TargetUserId;

    /**
     * If true, the local user is interested in the target user's presence and would like to receive notifications when
     * their presence changes. If false, they are not or no longer interested in the target user's presence.
     */
    const bool bInterested;
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(960893349, Redpoint::EOS::Presence, FPresenceInterestChange)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()