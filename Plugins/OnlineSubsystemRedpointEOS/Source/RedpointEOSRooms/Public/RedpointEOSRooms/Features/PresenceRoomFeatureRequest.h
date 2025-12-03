// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2629249321, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPresenceRoomFeatureRequest : public IRoomFeatureRequest<IPresenceRoomFeature>
{
private:
    bool bAllowJoinByPresenceValue;

public:
    FPresenceRoomFeatureRequest(bool bAllowJoinByPresence);
    UE_NONCOPYABLE(FPresenceRoomFeatureRequest);
    virtual ~FPresenceRoomFeatureRequest() override = default;

    /**
     * If true, players who can see the room via presence will be able to join it (via the Epic Games overlay).
     *
     * If false, and the room has the PublicAdvertisementRoomFeature, then players can still join the room because it's
     * public.
     *
     * If false, and the room has the InvitesRoomFeature, then players can only join via invites.
     *
     * If false and the room has neither of those other features, players won't be able to join the room at all.
     */
    bool AllowJoinByPresence() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2629249321, Redpoint::EOS::Rooms::Features, FPresenceRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()