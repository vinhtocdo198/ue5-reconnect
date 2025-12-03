// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Providers/RoomProvider.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3574048417, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Providers;

class FPartyTypeRoomFeatureImpl : public IPartyTypeRoomFeature
{
public:
    uint32 PartyTypeId;

    virtual uint32 GetPartyTypeId() const override;
    virtual bool GetPartyTypeIndicatesPresenceEnabled() const override;
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;

    static TSharedRef<FPartyTypeRoomFeatureSearchStateImpl> ConstructFeatureSearchState(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TMap<FString, FRoomAttribute> &FeatureAttributes);
    static TSharedRef<FPartyTypeRoomFeatureImpl> ConstructFeatureImplementation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room);
    static FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult
    UpdateFeatureImplementationDuringPostSynchronisation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FPartyTypeRoomFeatureImpl> &Feature,
        const TMap<FString, FRoomAttribute> &FeatureAttributes,
        const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes);
    static TMap<FString, FRoomAttribute> SerializeFeatureRequestToRoomAttributes(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedRef<FPartyTypeRoomFeatureRequest> &FeatureRequest);
    static FRoomProviderFeatureHandlerBase::FExclusivityKey GetRoomExclusivityKeyFromFeatureRequest(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomPtr &Room,
        const TSharedRef<FPartyTypeRoomFeatureRequest> &FeatureRequest);
    static FRoomProviderFeatureHandlerBase::FExclusivityKey GetRoomExclusivityKeyFromFeature(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FPartyTypeRoomFeatureImpl> &Feature);
    static FRoomProviderFeatureHandlerBase::FExclusivityKey GetRoomExclusivityKeyFromFeatureSearchState(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IPartyTypeRoomFeatureSearchState> &FeatureSearchState);
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3574048417, Redpoint::EOS::Rooms::Features, FPartyTypeRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()