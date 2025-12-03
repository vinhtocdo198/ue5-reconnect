// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/RoomSearchResultImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1726319936, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms::Providers;

FRoomSearchResultImpl::FRoomSearchResultImpl(
    const FRoomUserId &InBoundForLocalUser,
    const FRoomIdRef &InRoomId,
    const FName &InRoomNamespace,
    const FRoomOwnerRef &InRoomOwner,
    uint32 InRoomMaxMembers,
    const TMap<FName, TSharedRef<IRoomFeatureSearchStateBase>> &InRoomFeatureSearchStates,
    const TMap<FString, FRoomAttribute> &InRoomAttributes,
    const FRoomProviderReadOperationRef &InReadOperation,
    const TOptional<EOS_UI_EventId> &InAssociatedUiEventId)
    : BoundForLocalUser(InBoundForLocalUser)
    , RoomId(InRoomId)
    , RoomNamespace(InRoomNamespace)
    , RoomOwner(InRoomOwner)
    , RoomMaxMembers(InRoomMaxMembers)
    , RoomFeatureSearchStates(InRoomFeatureSearchStates)
    , RoomAttributes(InRoomAttributes)
    , ReadOperation(InReadOperation)
    , AssociatedUiEventId(InAssociatedUiEventId)
{
}

FRoomIdRef FRoomSearchResultImpl::GetRoomId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomId;
}

FName FRoomSearchResultImpl::GetRoomNamespace() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomNamespace;
}

FRoomOwnerRef FRoomSearchResultImpl::GetRoomOwner() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomOwner;
}

uint32 FRoomSearchResultImpl::GetRoomMaxMembers() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomMaxMembers;
}

const TMap<FString, FRoomAttribute> &FRoomSearchResultImpl::GetRoomAttributes() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomAttributes;
}

TOptional<EOS_UI_EventId> FRoomSearchResultImpl::GetAssociatedUiEventId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->AssociatedUiEventId;
}

TSet<FName> FRoomSearchResultImpl::GetRoomFeatureSearchStateNames() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    TSet<FName> Result;
    for (const auto &KV : this->RoomFeatureSearchStates)
    {
        Result.Add(KV.Key);
    }
    return Result;
}

TSharedPtr<Features::IRoomFeatureSearchStateBase> FRoomSearchResultImpl::GetRoomFeatureSearchStateBase(
    const FName &FeatureName) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->RoomFeatureSearchStates.Contains(FeatureName))
    {
        return this->RoomFeatureSearchStates[FeatureName];
    }
    else
    {
        return nullptr;
    }
}

FRoomUserId FRoomSearchResultImpl::GetBoundForLocalUser() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->BoundForLocalUser;
}

const FRoomProviderReadOperationRef &FRoomSearchResultImpl::GetReadOperation() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->ReadOperation;
}

}

REDPOINT_EOS_CODE_GUARD_END()