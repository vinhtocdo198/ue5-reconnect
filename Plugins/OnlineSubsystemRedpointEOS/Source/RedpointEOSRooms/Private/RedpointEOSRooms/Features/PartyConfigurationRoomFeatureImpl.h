// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Providers/RoomProvider.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(249413209, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Providers;

class FPartyConfigurationRoomFeatureImpl : public IPartyConfigurationRoomFeature
{
public:
    EPartyConfigurationRoomFeatureJoinRequestAction JoinRequestAction;
    EPartyConfigurationRoomFeaturePermissionType JoinByPresencePermissions;
    EPartyConfigurationRoomFeaturePermissionType SendInvitePermissions;
    FString Nickname;
    FString Description;
    FString Password;

    virtual EPartyConfigurationRoomFeatureJoinRequestAction GetJoinRequestAction() const override;
    virtual EPartyConfigurationRoomFeaturePermissionType GetJoinByPresencePermissions() const override;
    virtual EPartyConfigurationRoomFeaturePermissionType GetSendInvitePermissions() const override;
    virtual FString GetNickname() const override;
    virtual FString GetDescription() const override;
    virtual FString GetPassword() const override;

    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;

    static TSharedRef<FPartyConfigurationRoomFeatureSearchStateImpl> ConstructFeatureSearchState(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TMap<FString, FRoomAttribute> &FeatureAttributes);
    static TSharedRef<FPartyConfigurationRoomFeatureImpl> ConstructFeatureImplementation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room);
    static FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult
    UpdateFeatureImplementationDuringPostSynchronisation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FPartyConfigurationRoomFeatureImpl> &Feature,
        const TMap<FString, FRoomAttribute> &FeatureAttributes,
        const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes);
    static TMap<FString, FRoomAttribute> SerializeFeatureRequestToRoomAttributes(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedRef<FPartyConfigurationRoomFeatureRequest> &FeatureRequest);
    static TMap<FString, FRoomAttribute> SerializeFeatureRequestToRoomMemberAttributes(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedRef<FPartyConfigurationRoomFeatureRequest> &FeatureRequest);
    static bool ShouldKickReadyRoomMemberAfterJoin(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FPartyConfigurationRoomFeatureImpl> &Feature,
        FRoomUserId RoomMemberId,
        const TMap<FString, FRoomAttribute> &FeatureMemberAttributes);
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(249413209, Redpoint::EOS::Rooms::Features, FPartyConfigurationRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()