// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PresenceRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2322647541, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

bool FPresenceRoomFeatureImpl::AllowJoinFromPresence() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bAllowJoinFromPresence;
}

TSharedPtr<IRoomFeatureRequestBase> FPresenceRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FPresenceRoomFeatureRequest>(this->bAllowJoinFromPresence);
}

FRoomProviderFeatureHandlerBase::FExclusivityKey FPresenceRoomFeatureImpl::GetRoomExclusivityKeyFromFeatureRequest(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const TSharedPtr<IRoom> &Room,
    const TSharedRef<FPresenceRoomFeatureRequest> &FeatureRequest)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FRoomProviderFeatureHandlerBase::FExclusivityKey(
        TEXT("Presence"),
        FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::ReplaceExistingRoom);
}

FRoomProviderFeatureHandlerBase::FExclusivityKey FPresenceRoomFeatureImpl::GetRoomExclusivityKeyFromFeature(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const TSharedRef<IRoom> &Room,
    const TSharedRef<FPresenceRoomFeatureImpl> &Feature)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return FRoomProviderFeatureHandlerBase::FExclusivityKey(
        TEXT("Presence"),
        FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::ReplaceExistingRoom);
}

}

REDPOINT_EOS_CODE_GUARD_END()