// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Synchronisation/RoomMemberAttributeSynchronisationDifference.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1929713852, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

FRoomMemberAttributeSynchronisationDifference::FRoomMemberAttributeSynchronisationDifference(
    FRoomUserId InMemberUserId,
    FString InAttributeKey,
    EChangeType InChangeType,
    const FRoomAttribute &InOldAttributeValue,
    const FRoomAttribute &InNewAttributeValue)
    : MemberUserId(InMemberUserId)
    , AttributeKey(InAttributeKey)
    , ChangeType(InChangeType)
    , OldAttributeValue(InOldAttributeValue)
    , NewAttributeValue(InNewAttributeValue)
{
}

FName FRoomMemberAttributeSynchronisationDifference::GetDifferenceTypeStatic()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return "MemberAttribute";
}

FName FRoomMemberAttributeSynchronisationDifference::GetDifferenceType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return GetDifferenceTypeStatic();
}

FRoomUserId FRoomMemberAttributeSynchronisationDifference::GetMemberUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->MemberUserId;
}

FString FRoomMemberAttributeSynchronisationDifference::GetAttributeKey() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->AttributeKey;
}

FRoomMemberAttributeSynchronisationDifference::EChangeType FRoomMemberAttributeSynchronisationDifference::
    GetChangeType() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->ChangeType;
}

const FRoomAttribute &FRoomMemberAttributeSynchronisationDifference::GetOldAttributeValue() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->OldAttributeValue;
}

const FRoomAttribute &FRoomMemberAttributeSynchronisationDifference::GetNewAttributeValue() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return this->NewAttributeValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()