// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Synchronisation/RoomMemberInfoSynchronisationDifference.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(983476630, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

FRoomMemberInfoSynchronisationDifference::FRoomMemberInfoSynchronisationDifference(FRoomUserId InMemberUserId)
    : MemberUserId(InMemberUserId)
{
}

FName FRoomMemberInfoSynchronisationDifference::GetDifferenceTypeStatic()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return "MemberInfo";
}

FName FRoomMemberInfoSynchronisationDifference::GetDifferenceType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return GetDifferenceTypeStatic();
}

FRoomUserId FRoomMemberInfoSynchronisationDifference::GetMemberUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->MemberUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()