// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(745617141, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::UserCache;

class FPresenceSystemRemoteUserState
{
public:
    TSet<FName> InterestedSystems;
    UE::Online::FAccountId UserId;
    FUserInfoPtr UserInfo;

    TOptional<FPresenceUserState> FullPresence;
    FDateTime DateLastPresenceCached;

    TOptional<FPresenceUserState> PartialPresence;
    TMap<TSharedRef<IRuntimePlatformPresenceService>, EPresenceUserActivityState> PartialRuntimeActivityState;

    FPresenceSystemRemoteUserState(const UE::Online::FAccountId &InUserId);

    void RecomputePartialPresence();

    bool IsBestPresenceFullPresence() const;
    const TOptional<FPresenceUserState> &GetBestPresence() const;
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(745617141, Redpoint::EOS::Presence, FPresenceSystemRemoteUserState)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()