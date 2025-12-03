// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Synchronisation/RoomFeatureSynchronisationDifference.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4071186716, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

FRoomFeatureSynchronisationDifference::FRoomFeatureSynchronisationDifference(
    FName InFeatureName,
    EChangeType InChangeType)
    : FeatureName(InFeatureName)
    , ChangeType(InChangeType)
{
}

FName FRoomFeatureSynchronisationDifference::GetDifferenceTypeStatic()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return "Feature";
}

FName FRoomFeatureSynchronisationDifference::GetDifferenceType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return GetDifferenceTypeStatic();
}

FName FRoomFeatureSynchronisationDifference::GetFeatureName() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->FeatureName;
}

FRoomFeatureSynchronisationDifference::EChangeType FRoomFeatureSynchronisationDifference::GetChangeType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->ChangeType;
}

}

REDPOINT_EOS_CODE_GUARD_END()