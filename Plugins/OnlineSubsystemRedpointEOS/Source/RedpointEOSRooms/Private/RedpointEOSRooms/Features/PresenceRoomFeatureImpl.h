// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/PresenceRoomFeature.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Providers/RoomProvider.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2322647541, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Providers;

class FPresenceRoomFeatureImpl : public IPresenceRoomFeature
{
public:
    bool bAllowJoinFromPresence;

    virtual bool AllowJoinFromPresence() const override;
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;

    static FRoomProviderFeatureHandlerBase::FExclusivityKey GetRoomExclusivityKeyFromFeatureRequest(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedPtr<IRoom> &Room,
        const TSharedRef<FPresenceRoomFeatureRequest> &FeatureRequest);
    static FRoomProviderFeatureHandlerBase::FExclusivityKey GetRoomExclusivityKeyFromFeature(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoom> &Room,
        const TSharedRef<FPresenceRoomFeatureImpl> &Feature);
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2322647541, Redpoint::EOS::Rooms::Features, FPresenceRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()