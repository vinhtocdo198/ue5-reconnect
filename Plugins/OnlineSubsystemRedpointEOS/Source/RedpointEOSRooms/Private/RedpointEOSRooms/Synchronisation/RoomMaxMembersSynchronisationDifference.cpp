// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Synchronisation/RoomMaxMembersSynchronisationDifference.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2565147049, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

FRoomMaxMembersSynchronisationDifference::FRoomMaxMembersSynchronisationDifference(uint32 InNewMaxMembers)
    : NewMaxMembers(InNewMaxMembers)
{
}

FName FRoomMaxMembersSynchronisationDifference::GetDifferenceTypeStatic()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return "MaxMembers";
}

FName FRoomMaxMembersSynchronisationDifference::GetDifferenceType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return GetDifferenceTypeStatic();
}

uint32 FRoomMaxMembersSynchronisationDifference::GetNewMaxMembers() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->NewMaxMembers;
}

}

REDPOINT_EOS_CODE_GUARD_END()