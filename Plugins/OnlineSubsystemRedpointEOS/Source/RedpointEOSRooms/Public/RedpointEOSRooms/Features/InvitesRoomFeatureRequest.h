// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4224030567, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FInvitesRoomFeatureRequest : public IRoomFeatureRequest<IInvitesRoomFeature>
{
private:
    bool bRejoinAfterKickRequiresInviteValue;

public:
    FInvitesRoomFeatureRequest(bool bRejoinAfterKickRequiresInvite);
    UE_NONCOPYABLE(FInvitesRoomFeatureRequest);
    virtual ~FInvitesRoomFeatureRequest() override = default;

    bool RejoinAfterKickRequiresInvite() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(4224030567, Redpoint::EOS::Rooms::Features, FInvitesRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()