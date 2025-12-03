// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/RoomImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3293798267, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms::Providers;

FRoomImpl::FRoomImpl(const FRoomUserId &InBoundForLocalUser, const FRoomIdRef &InRoomId)
    : BoundForLocalUser(InBoundForLocalUser)
    , RoomId(InRoomId)
    , RoomOwner()
    , RoomNamespace(NAME_None)
    , RoomMaxMembers(UINT32_MAX)
    , RoomFeatures()
    , RoomAttributes()
    , RoomMembers()
    , RoomUnreadyMembersFirstSeenTimestamp()
    , RoomMembersBeingAutomaticallyKicked()
    , OnRoomMemberJoinedValue()
    , OnRoomMemberLeftValue()
    , OnRoomMemberUpdatedValue()
    , OnRoomMemberPromotedValue()
{
}

const FRoomIdRef &FRoomImpl::GetRoomId() const
{
    return this->RoomId;
}

FName FRoomImpl::GetRoomNamespace() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    checkf(!this->RoomNamespace.IsNone(), TEXT("GetRoomNamespace called prior to initial room synchronisation!"));
    return this->RoomNamespace;
}

FRoomOwnerRef FRoomImpl::GetRoomOwner() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // @note: This can technically assert if GetRoomOwner is
    // called before it is synchronised for the first time
    // in Synchronise, but all public facing APIs for rooms
    // ensure Synchronise is called before the room instance is
    // returned to external code.
    return this->RoomOwner.ToSharedRef();
}

uint32 FRoomImpl::GetRoomMaxMembers() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomMaxMembers;
}

const TMap<FString, FRoomAttribute> &FRoomImpl::GetRoomAttributes() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomAttributes;
}

TSet<FName> FRoomImpl::GetRoomFeatureNames() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    TSet<FName> Result;
    for (const auto &KV : this->RoomFeatures)
    {
        Result.Add(KV.Key);
    }
    return Result;
}

TSharedPtr<Features::IRoomFeature> FRoomImpl::GetRoomFeatureBase(const FName &FeatureName) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->RoomFeatures.Contains(FeatureName))
    {
        return this->RoomFeatures[FeatureName];
    }
    else
    {
        return nullptr;
    }
}

const TArray<FRoomMemberRef> &FRoomImpl::GetMembers() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomMembers;
}

IRoom::FOnRoomMemberJoined &FRoomImpl::OnRoomMemberJoined()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomMemberJoinedValue;
}

IRoom::FOnRoomMemberLeft &FRoomImpl::OnRoomMemberLeft()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomMemberLeftValue;
}

IRoom::FOnRoomMemberUpdated &FRoomImpl::OnRoomMemberUpdated()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomMemberUpdatedValue;
}

IRoom::FOnRoomMemberPromoted &FRoomImpl::OnRoomMemberPromoted()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomMemberPromotedValue;
}

FRoomUserId FRoomImpl::GetBoundForLocalUser() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->BoundForLocalUser;
}

void FRoomImpl::RemoveAllRoomFeatures()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    this->RoomFeatures.Empty();
}

}

REDPOINT_EOS_CODE_GUARD_END()