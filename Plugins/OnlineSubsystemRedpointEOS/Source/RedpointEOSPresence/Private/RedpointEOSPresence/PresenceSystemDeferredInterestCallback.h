// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2256700948, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Presence;

class FPresenceSystemDeferredInterestCallback
{
public:
    FIdentityUserId LocalUserId;
    TSet<FIdentityUserId> WaitingTargetUserIds;
    IPresenceSystem::FOnUpdatePresenceInterestComplete CompletionDelegate;

    FPresenceSystemDeferredInterestCallback(
        const FIdentityUserId &InLocalUserId,
        const TSet<FIdentityUserId> &InWaitingTargetUserIds,
        const IPresenceSystem::FOnUpdatePresenceInterestComplete &InCompletionDelegate);
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(2256700948, Redpoint::EOS::Presence, FPresenceSystemDeferredInterestCallback)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()