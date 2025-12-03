// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceSystemRemoteUserState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(745617141, Redpoint::EOS::Presence)
{

FPresenceSystemRemoteUserState::FPresenceSystemRemoteUserState(const UE::Online::FAccountId &InUserId)
    : InterestedSystems()
    , UserId(InUserId)
    , UserInfo()
    , FullPresence()
    , DateLastPresenceCached(FDateTime::MinValue())
    , PartialPresence()
    , PartialRuntimeActivityState()
{
}

void FPresenceSystemRemoteUserState::RecomputePartialPresence()
{
    auto ActivityState = EPresenceUserActivityState::Offline;
    for (const auto &KV : this->PartialRuntimeActivityState)
    {
        // Do Not Disturb beats Offline.
        if (KV.Value == EPresenceUserActivityState::DoNotDisturb &&
            ActivityState == EPresenceUserActivityState::Offline)
        {
            ActivityState = KV.Value;
        }
        // Idle beats Offline and Do Not Disturb.
        if (KV.Value == EPresenceUserActivityState::Idle && (ActivityState == EPresenceUserActivityState::Offline ||
                                                             ActivityState == EPresenceUserActivityState::DoNotDisturb))
        {
            ActivityState = KV.Value;
        }
        // Online beats anything.
        if (KV.Value == EPresenceUserActivityState::Online)
        {
            ActivityState = KV.Value;
            break;
        }
    }

    if (!this->PartialPresence.IsSet())
    {
        this->PartialPresence = FPresenceUserState(this->UserId);
    }

    this->PartialPresence->ActivityState = ActivityState;
    this->PartialPresence->bIsPlayingThisGame = false;
}

bool FPresenceSystemRemoteUserState::IsBestPresenceFullPresence() const
{
    if (this->FullPresence.IsSet() && this->FullPresence->ActivityState != EPresenceUserActivityState::Offline)
    {
        return true;
    }

    // We prioritize partial presence from the runtime platform if the full presence is "Offline" (even if set, since
    // that might just mean that user isn't playing this particular game and doesn't have a presence lobby registered).
    if (this->PartialPresence.IsSet() && this->PartialPresence->ActivityState != EPresenceUserActivityState::Offline)
    {
        return false;
    }

    return this->FullPresence.IsSet();
}

const TOptional<FPresenceUserState> &FPresenceSystemRemoteUserState::GetBestPresence() const
{
    return this->IsBestPresenceFullPresence() ? this->FullPresence : this->PartialPresence;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()