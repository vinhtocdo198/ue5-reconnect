// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Synchronisation/RoomOwnerSynchronisationDifference.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3679356872, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

FRoomOwnerSynchronisationDifference::FRoomOwnerSynchronisationDifference(FRoomOwnerRef InNewOwner)
    : NewOwner(InNewOwner)
{
}

FName FRoomOwnerSynchronisationDifference::GetDifferenceTypeStatic()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return "Owner";
}

FName FRoomOwnerSynchronisationDifference::GetDifferenceType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return GetDifferenceTypeStatic();
}

FRoomOwnerRef FRoomOwnerSynchronisationDifference::GetNewOwner() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->NewOwner;
}

}

REDPOINT_EOS_CODE_GUARD_END()