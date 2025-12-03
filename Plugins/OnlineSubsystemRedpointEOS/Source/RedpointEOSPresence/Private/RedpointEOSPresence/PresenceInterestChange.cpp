// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceInterestChange.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(960893349, Redpoint::EOS::Presence)
{

FPresenceInterestChange::FPresenceInterestChange(const FIdentityUserId &InTargetUserId, bool bInInterested)
    : TargetUserId(InTargetUserId)
    , bInterested(bInInterested)
{
}

FPresenceInterestChange::FPresenceInterestChange(const FPresenceInterestChange &InInterestChange)
    : TargetUserId(InInterestChange.TargetUserId)
    , bInterested(InInterestChange.bInterested)
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()