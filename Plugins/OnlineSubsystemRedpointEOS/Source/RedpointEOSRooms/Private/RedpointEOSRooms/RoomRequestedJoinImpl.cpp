// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomRequestedJoinImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4026645771, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

FRoomRequestedJoinImpl::FRoomRequestedJoinImpl(
    const FRoomUserId &InActorUserId,
    const FRoomSearchResultRef &InRoomSearchResult)
    : ActorUserId(InActorUserId)
    , RoomSearchResult(InRoomSearchResult)
    , RoomInvite(nullptr)
{
}

FRoomRequestedJoinImpl::FRoomRequestedJoinImpl(const FRoomInviteRef &InRoomInvite)
    : ActorUserId(InRoomInvite->GetRoomInviteRecipientUserId())
    , RoomSearchResult(InRoomInvite->GetRoomInviteSearchResult())
    , RoomInvite(InRoomInvite)
{
}

FRoomUserId FRoomRequestedJoinImpl::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->ActorUserId;
}

FRoomSearchResultRef FRoomRequestedJoinImpl::GetRoomSearchResult() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomSearchResult;
}

FRoomInvitePtr FRoomRequestedJoinImpl::GetRoomInvite() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomInvite;
}

}

REDPOINT_EOS_CODE_GUARD_END()