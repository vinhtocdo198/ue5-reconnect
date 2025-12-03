// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureSearchStateImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1130234379, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FPublicMemberListRoomFeatureSearchStateImpl::FPublicMemberListRoomFeatureSearchStateImpl()
    : MaxMembersValue()
    , CurrentMembersValue()
{
}

uint32 FPublicMemberListRoomFeatureSearchStateImpl::MaxMembers() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->MaxMembersValue;
}

uint32 FPublicMemberListRoomFeatureSearchStateImpl::AvailableSlots() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->MaxMembersValue - static_cast<uint32>(this->CurrentMembersValue.Num());
}

const TArray<FRoomUserId> &FPublicMemberListRoomFeatureSearchStateImpl::CurrentMembers() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->CurrentMembersValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()