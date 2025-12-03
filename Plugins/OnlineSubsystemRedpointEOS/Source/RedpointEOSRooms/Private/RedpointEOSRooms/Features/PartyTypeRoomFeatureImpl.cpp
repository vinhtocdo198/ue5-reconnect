// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3574048417, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

uint32 FPartyTypeRoomFeatureImpl::GetPartyTypeId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->PartyTypeId;
}

bool FPartyTypeRoomFeatureImpl::GetPartyTypeIndicatesPresenceEnabled() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->PartyTypeId == Redpoint::EOS::Rooms::Features::PartyTypePrimaryPartyId;
}

TSharedPtr<IRoomFeatureRequestBase> FPartyTypeRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FPartyTypeRoomFeatureRequest>(this->PartyTypeId);
}

TSharedRef<FPartyTypeRoomFeatureSearchStateImpl> FPartyTypeRoomFeatureImpl::ConstructFeatureSearchState(
    const FRoomProviderReadOperationRef &ReadOp,
    const TSharedRef<FRoomProvider> &RoomProvider,
    const TMap<FString, FRoomAttribute> &FeatureAttributes)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    auto SearchState = MakeShared<FPartyTypeRoomFeatureSearchStateImpl>();
    auto PartyTypeIdAttribute = FeatureAttributes.Find(TEXT("PartyTypeId"));
    if (PartyTypeIdAttribute != nullptr && PartyTypeIdAttribute->Type() == FRoomAttribute::EType::Int64)
    {
        SearchState->PartyTypeIdValue = (uint32)PartyTypeIdAttribute->GetInt64();
    }
    return SearchState;
}

TSharedRef<FPartyTypeRoomFeatureImpl> FPartyTypeRoomFeatureImpl::ConstructFeatureImplementation(
    const FRoomProviderReadOperationRef &ReadOp,
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FPartyTypeRoomFeatureImpl>();
}

FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult FPartyTypeRoomFeatureImpl::
    UpdateFeatureImplementationDuringPostSynchronisation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FPartyTypeRoomFeatureImpl> &Feature,
        const TMap<FString, FRoomAttribute> &FeatureAttributes,
        const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    bool bUpdated = false;

    auto PartyTypeIdAttribute = FeatureAttributes.Find(TEXT("PartyTypeId"));
    if (PartyTypeIdAttribute != nullptr && PartyTypeIdAttribute->Type() == FRoomAttribute::EType::Int64)
    {
        bUpdated = bUpdated || (Feature->PartyTypeId != (uint32)PartyTypeIdAttribute->GetInt64());
        Feature->PartyTypeId = (uint32)PartyTypeIdAttribute->GetInt64();
    }

    return FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult(
        FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::ERequireRoomUpdate::No,
        bUpdated ? FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::Yes
                 : FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::No);
}

TMap<FString, FRoomAttribute> FPartyTypeRoomFeatureImpl::SerializeFeatureRequestToRoomAttributes(
    const FRoomProviderWriteOperationRef &WriteOp,
    const TSharedRef<FPartyTypeRoomFeatureRequest> &FeatureRequest)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return TMap<FString, FRoomAttribute>{
        {TEXT("PartyTypeId"), (int64)FeatureRequest->PartyTypeId()},
    };
}

FRoomProviderFeatureHandlerBase::FExclusivityKey FPartyTypeRoomFeatureImpl::GetRoomExclusivityKeyFromFeatureRequest(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomPtr &Room,
    const TSharedRef<FPartyTypeRoomFeatureRequest> &FeatureRequest)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FRoomProviderFeatureHandlerBase::FExclusivityKey(
        FString::Printf(TEXT("PartyTypeId:%u"), FeatureRequest->PartyTypeId()),
        FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::ReplaceExistingRoom);
}

FRoomProviderFeatureHandlerBase::FExclusivityKey FPartyTypeRoomFeatureImpl::GetRoomExclusivityKeyFromFeature(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomRef &Room,
    const TSharedRef<FPartyTypeRoomFeatureImpl> &Feature)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FRoomProviderFeatureHandlerBase::FExclusivityKey(
        FString::Printf(TEXT("PartyTypeId:%u"), Feature->GetPartyTypeId()),
        FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::ReplaceExistingRoom);
}

FRoomProviderFeatureHandlerBase::FExclusivityKey FPartyTypeRoomFeatureImpl::GetRoomExclusivityKeyFromFeatureSearchState(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const TSharedRef<IPartyTypeRoomFeatureSearchState> &FeatureSearchState)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FRoomProviderFeatureHandlerBase::FExclusivityKey(
        FString::Printf(TEXT("PartyTypeId:%u"), FeatureSearchState->PartyTypeId()),
        FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::ReplaceExistingRoom);
}

}

REDPOINT_EOS_CODE_GUARD_END()