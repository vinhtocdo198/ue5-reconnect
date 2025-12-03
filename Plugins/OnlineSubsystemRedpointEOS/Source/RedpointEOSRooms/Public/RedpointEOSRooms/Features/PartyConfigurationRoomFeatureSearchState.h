// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3885394703, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API IPartyConfigurationRoomFeatureSearchState
    : public IRoomFeatureSearchState<IPartyConfigurationRoomFeature>
{
public:
    virtual EPartyConfigurationRoomFeatureJoinRequestAction GetJoinRequestAction() const = 0;
    virtual EPartyConfigurationRoomFeaturePermissionType GetJoinByPresencePermissions() const = 0;
    virtual EPartyConfigurationRoomFeaturePermissionType GetSendInvitePermissions() const = 0;
    virtual FString GetNickname() const = 0;
    virtual FString GetDescription() const = 0;
    virtual bool IsPasswordRequired() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3885394703, Redpoint::EOS::Rooms::Features, IPartyConfigurationRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()