// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/AliasRoomFeature.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Providers/RoomProvider.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1395500338, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Providers;

class FAliasRoomFeatureImpl : public IAliasRoomFeature
{
public:
    FName AliasValue;

    virtual const FName &Alias() const override;
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;

    static TSharedRef<FAliasRoomFeatureSearchStateImpl> ConstructFeatureSearchState(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TMap<FString, FRoomAttribute> &FeatureAttributes);
    static TSharedRef<FAliasRoomFeatureImpl> ConstructFeatureImplementation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room);
    static FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult
    UpdateFeatureImplementationDuringPostSynchronisation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FAliasRoomFeatureImpl> &Feature,
        const TMap<FString, FRoomAttribute> &FeatureAttributes,
        const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes);
    static TMap<FString, FRoomAttribute> SerializeFeatureRequestToRoomAttributes(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedRef<FAliasRoomFeatureRequest> &FeatureRequest);
    static FRoomProviderFeatureHandlerBase::FExclusivityKey GetRoomExclusivityKeyFromFeatureRequest(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomPtr &Room,
        const TSharedRef<FAliasRoomFeatureRequest> &FeatureRequest);
    static FRoomProviderFeatureHandlerBase::FExclusivityKey GetRoomExclusivityKeyFromFeature(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FAliasRoomFeatureImpl> &Feature);
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1395500338, Redpoint::EOS::Rooms::Features, FAliasRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()