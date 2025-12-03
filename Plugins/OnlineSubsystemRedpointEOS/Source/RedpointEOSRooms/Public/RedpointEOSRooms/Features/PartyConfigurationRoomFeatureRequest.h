// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1899928368, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPartyConfigurationRoomFeatureRequest
    : public IRoomFeatureRequest<IPartyConfigurationRoomFeature>
{
private:
    EPartyConfigurationRoomFeatureJoinRequestAction JoinRequestAction;
    EPartyConfigurationRoomFeaturePermissionType JoinByPresencePermissions;
    EPartyConfigurationRoomFeaturePermissionType SendInvitePermissions;
    FString Nickname;
    FString Description;
    FString Password;

public:
    FPartyConfigurationRoomFeatureRequest(
        EPartyConfigurationRoomFeatureJoinRequestAction InJoinRequestAction,
        EPartyConfigurationRoomFeaturePermissionType InJoinByPresencePermissions,
        EPartyConfigurationRoomFeaturePermissionType InSendInvitePermissions,
        const FString &InNickname,
        const FString &InDescription,
        const FString &InPassword);
    FPartyConfigurationRoomFeatureRequest(const FString &InClientPassword);
    UE_NONCOPYABLE(FPartyConfigurationRoomFeatureRequest);
    ~FPartyConfigurationRoomFeatureRequest() override = default;

    virtual bool IsValid() const override;

    EPartyConfigurationRoomFeatureJoinRequestAction GetJoinRequestAction() const;
    EPartyConfigurationRoomFeaturePermissionType GetJoinByPresencePermissions() const;
    EPartyConfigurationRoomFeaturePermissionType GetSendInvitePermissions() const;
    const FString &GetNickname() const;
    const FString &GetDescription() const;
    const FString &GetPassword() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1899928368, Redpoint::EOS::Rooms::Features, FPartyConfigurationRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()