// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Synchronisation/RoomMemberSynchronisationDifference.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3146034446, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

FRoomMemberSynchronisationDifference::FRoomMemberSynchronisationDifference(
    FRoomUserId InMemberUserId,
    EChangeType InChangeType,
    bool InIsForBoundUser)
    : MemberUserId(InMemberUserId)
    , ChangeType(InChangeType)
    , bIsForBoundUser(InIsForBoundUser)
{
}

FName FRoomMemberSynchronisationDifference::GetDifferenceTypeStatic()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return "Member";
}

FName FRoomMemberSynchronisationDifference::GetDifferenceType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return GetDifferenceTypeStatic();
}

FRoomUserId FRoomMemberSynchronisationDifference::GetMemberUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->MemberUserId;
}

FRoomMemberSynchronisationDifference::EChangeType FRoomMemberSynchronisationDifference::GetChangeType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->ChangeType;
}

bool FRoomMemberSynchronisationDifference::IsForBoundUser() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->bIsForBoundUser;
}

}

REDPOINT_EOS_CODE_GUARD_END()