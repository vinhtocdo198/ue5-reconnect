// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3356681519, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPartyConfigurationRoomFeatureSearchStateImpl
    : public IPartyConfigurationRoomFeatureSearchState
{
public:
    EPartyConfigurationRoomFeatureJoinRequestAction JoinRequestAction;
    EPartyConfigurationRoomFeaturePermissionType JoinByPresencePermissions;
    EPartyConfigurationRoomFeaturePermissionType SendInvitePermissions;
    FString Nickname;
    FString Description;
    bool bIsPasswordRequired;

    FPartyConfigurationRoomFeatureSearchStateImpl();
    UE_NONCOPYABLE(FPartyConfigurationRoomFeatureSearchStateImpl);
    virtual ~FPartyConfigurationRoomFeatureSearchStateImpl() override = default;

    virtual EPartyConfigurationRoomFeatureJoinRequestAction GetJoinRequestAction() const override;
    virtual EPartyConfigurationRoomFeaturePermissionType GetJoinByPresencePermissions() const override;
    virtual EPartyConfigurationRoomFeaturePermissionType GetSendInvitePermissions() const override;
    virtual FString GetNickname() const override;
    virtual FString GetDescription() const override;
    virtual bool IsPasswordRequired() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3356681519, Redpoint::EOS::Rooms::Features, FPartyConfigurationRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()