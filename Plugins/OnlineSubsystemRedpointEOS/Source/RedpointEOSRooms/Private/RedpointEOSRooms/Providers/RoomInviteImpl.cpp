// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/RoomInviteImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(529624270, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;

FRoomInviteImpl::FRoomInviteImpl(
    const FRoomInviteId &InRoomInviteId,
    const FRoomUserId &InRoomInviteSenderUserId,
    const FRoomUserId &InRoomInviteRecipientUserId,
    const FRoomSearchResultRef &InRoomSearchResult)
    : RoomInviteId(InRoomInviteId)
    , RoomInviteSenderUserId(InRoomInviteSenderUserId)
    , RoomInviteRecipientUserId(InRoomInviteRecipientUserId)
    , RoomSearchResult(InRoomSearchResult)
{
}

FRoomInviteId FRoomInviteImpl::GetRoomInviteId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomInviteId;
}

FRoomUserId FRoomInviteImpl::GetRoomInviteSenderUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomInviteSenderUserId;
}

FRoomUserId FRoomInviteImpl::GetRoomInviteRecipientUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomInviteRecipientUserId;
}

TSharedRef<IRoomSearchResult> FRoomInviteImpl::GetRoomInviteSearchResult() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomSearchResult;
}

}

REDPOINT_EOS_CODE_GUARD_END()