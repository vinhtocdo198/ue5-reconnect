// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/RoomMemberImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(711375327, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms::Providers;

FRoomMemberImpl::FRoomMemberImpl(const FRoomUserId &InUserId)
    : UserId(InUserId)
    , RoomMemberAttributes()
{
}

TMap<FString, FRoomAttribute> &FRoomMemberImpl::GetMutableAttributes()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomMemberAttributes;
}

const FRoomUserId &FRoomMemberImpl::GetUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->UserId;
}

const TMap<FString, FRoomAttribute> &FRoomMemberImpl::GetAttributes() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomMemberAttributes;
}

}

REDPOINT_EOS_CODE_GUARD_END()