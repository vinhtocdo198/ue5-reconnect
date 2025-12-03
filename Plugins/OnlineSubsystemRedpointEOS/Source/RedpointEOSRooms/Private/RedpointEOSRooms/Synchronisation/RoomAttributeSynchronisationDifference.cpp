// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Synchronisation/RoomAttributeSynchronisationDifference.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1699210340, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

FRoomAttributeSynchronisationDifference::FRoomAttributeSynchronisationDifference(
    FString InAttributeKey,
    EChangeType InChangeType,
    const FRoomAttribute &InOldAttributeValue,
    const FRoomAttribute &InNewAttributeValue)
    : AttributeKey(InAttributeKey)
    , ChangeType(InChangeType)
    , OldAttributeValue(InOldAttributeValue)
    , NewAttributeValue(InNewAttributeValue)
{
}

FName FRoomAttributeSynchronisationDifference::GetDifferenceTypeStatic()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return "Attribute";
}

FName FRoomAttributeSynchronisationDifference::GetDifferenceType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return GetDifferenceTypeStatic();
}

FString FRoomAttributeSynchronisationDifference::GetAttributeKey() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->AttributeKey;
}

FRoomAttributeSynchronisationDifference::EChangeType FRoomAttributeSynchronisationDifference::GetChangeType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->ChangeType;
}

const FRoomAttribute &FRoomAttributeSynchronisationDifference::GetOldAttributeValue() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->OldAttributeValue;
}

const FRoomAttribute &FRoomAttributeSynchronisationDifference::GetNewAttributeValue() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->NewAttributeValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()