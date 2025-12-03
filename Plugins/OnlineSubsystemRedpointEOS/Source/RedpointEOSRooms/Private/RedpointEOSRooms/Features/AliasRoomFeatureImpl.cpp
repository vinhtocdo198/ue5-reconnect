// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/AliasRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1395500338, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

const FName &FAliasRoomFeatureImpl::Alias() const
{
    return this->AliasValue;
}

TSharedPtr<IRoomFeatureRequestBase> FAliasRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FAliasRoomFeatureRequest>(this->AliasValue);
}

TSharedRef<FAliasRoomFeatureSearchStateImpl> FAliasRoomFeatureImpl::ConstructFeatureSearchState(
    const FRoomProviderReadOperationRef &ReadOp,
    const TSharedRef<FRoomProvider> &RoomProvider,
    const TMap<FString, FRoomAttribute> &FeatureAttributes)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FAliasRoomFeatureSearchStateImpl>();
}

TSharedRef<FAliasRoomFeatureImpl> FAliasRoomFeatureImpl::ConstructFeatureImplementation(
    const FRoomProviderReadOperationRef &ReadOp,
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FAliasRoomFeatureImpl>();
}

FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult FAliasRoomFeatureImpl::
    UpdateFeatureImplementationDuringPostSynchronisation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FAliasRoomFeatureImpl> &Feature,
        const TMap<FString, FRoomAttribute> &FeatureAttributes,
        const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    bool bUpdated = false;

    auto ThisUserAttributes = FeatureMemberAttributes.Find(Room->GetBoundForLocalUser());
    if (ThisUserAttributes != nullptr)
    {
        auto AliasAttribute = ThisUserAttributes->Find(TEXT("Alias"));
        if (AliasAttribute != nullptr)
        {
            bUpdated = bUpdated || (Feature->AliasValue != FName(*AliasAttribute->GetString()));
            Feature->AliasValue = FName(*AliasAttribute->GetString());
        }
    }

    return FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult(
        FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::ERequireRoomUpdate::No,
        bUpdated ? FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::Yes
                 : FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::No);
}

TMap<FString, FRoomAttribute> FAliasRoomFeatureImpl::SerializeFeatureRequestToRoomAttributes(
    const FRoomProviderWriteOperationRef &WriteOp,
    const TSharedRef<FAliasRoomFeatureRequest> &FeatureRequest)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return TMap<FString, FRoomAttribute>{
        {TEXT("Alias"), FeatureRequest->Alias().ToString()},
    };
}

FRoomProviderFeatureHandlerBase::FExclusivityKey FAliasRoomFeatureImpl::GetRoomExclusivityKeyFromFeatureRequest(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomPtr &Room,
    const TSharedRef<FAliasRoomFeatureRequest> &FeatureRequest)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FRoomProviderFeatureHandlerBase::FExclusivityKey(
        FString::Printf(TEXT("Alias:%s"), *FeatureRequest->Alias().ToString()),
        FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::RejectNewRoom);
}

FRoomProviderFeatureHandlerBase::FExclusivityKey FAliasRoomFeatureImpl::GetRoomExclusivityKeyFromFeature(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomRef &Room,
    const TSharedRef<FAliasRoomFeatureImpl> &Feature)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FRoomProviderFeatureHandlerBase::FExclusivityKey(
        FString::Printf(TEXT("Alias:%s"), *Feature->Alias().ToString()),
        FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::RejectNewRoom);
}

}

REDPOINT_EOS_CODE_GUARD_END()