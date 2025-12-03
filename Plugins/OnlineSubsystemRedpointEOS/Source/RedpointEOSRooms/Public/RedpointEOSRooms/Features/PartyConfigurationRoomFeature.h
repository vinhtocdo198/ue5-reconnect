// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3188680639, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

enum EPartyConfigurationRoomFeatureJoinRequestAction
{
    Manual,
    AutoApprove,
    AutoReject,
};

enum EPartyConfigurationRoomFeaturePermissionType
{
    Noone,
    Leader,
    Friends,
    Anyone,
};

class REDPOINTEOSROOMS_API IPartyConfigurationRoomFeature : public IRoomFeature
{
public:
    static FName GetFeatureName();

    virtual EPartyConfigurationRoomFeatureJoinRequestAction GetJoinRequestAction() const = 0;
    virtual EPartyConfigurationRoomFeaturePermissionType GetJoinByPresencePermissions() const = 0;
    virtual EPartyConfigurationRoomFeaturePermissionType GetSendInvitePermissions() const = 0;
    virtual FString GetNickname() const = 0;
    virtual FString GetDescription() const = 0;
    virtual FString GetPassword() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3188680639, Redpoint::EOS::Rooms::Features, EPartyConfigurationRoomFeatureJoinRequestAction)
REDPOINT_EOS_FILE_NS_EXPORT(3188680639, Redpoint::EOS::Rooms::Features, EPartyConfigurationRoomFeaturePermissionType)
REDPOINT_EOS_FILE_NS_EXPORT(3188680639, Redpoint::EOS::Rooms::Features, IPartyConfigurationRoomFeature)
}

REDPOINT_EOS_CODE_GUARD_END()