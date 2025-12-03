// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceSystemDeferredInterestCallback.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2256700948, Redpoint::EOS::Presence)
{

FPresenceSystemDeferredInterestCallback::FPresenceSystemDeferredInterestCallback(
    const FIdentityUserId &InLocalUserId,
    const TSet<FIdentityUserId> &InWaitingTargetUserIds,
    const IPresenceSystem::FOnUpdatePresenceInterestComplete &InCompletionDelegate)
    : LocalUserId(InLocalUserId)
    , WaitingTargetUserIds(InWaitingTargetUserIds)
    , CompletionDelegate(InCompletionDelegate)
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()