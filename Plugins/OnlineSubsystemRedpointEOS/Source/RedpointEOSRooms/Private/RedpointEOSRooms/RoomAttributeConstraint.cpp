// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomAttributeConstraint.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(671663452, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::API::LobbySessionsShared;

FRoomAttributeConstraint::FRoomAttributeConstraint(const FRoomAttribute &InAttribute, EConstraintType InConstraintType)
    : _Attribute(InAttribute)
    , _ConstraintType(InConstraintType)
{
    _Attribute = InAttribute;
    _ConstraintType = InConstraintType;
}

EOS_EComparisonOp FRoomAttributeConstraint::GetComparisonOp() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    switch (this->_ConstraintType)
    {
    case EConstraintType::Equal:
        return EOS_EComparisonOp::EOS_CO_EQUAL;
    case EConstraintType::NotEqual:
        return EOS_EComparisonOp::EOS_CO_NOTEQUAL;
    case EConstraintType::GreaterThan:
        return EOS_EComparisonOp::EOS_CO_GREATERTHAN;
    case EConstraintType::GreaterThanOrEqual:
        return EOS_EComparisonOp::EOS_CO_GREATERTHANOREQUAL;
    case EConstraintType::LessThan:
        return EOS_EComparisonOp::EOS_CO_LESSTHAN;
    case EConstraintType::LessThanOrEqual:
        return EOS_EComparisonOp::EOS_CO_LESSTHANOREQUAL;
    case EConstraintType::Distance:
        return EOS_EComparisonOp::EOS_CO_DISTANCE;
    case EConstraintType::AnyOf:
        return EOS_EComparisonOp::EOS_CO_ANYOF;
    case EConstraintType::NotAnyOf:
        return EOS_EComparisonOp::EOS_CO_NOTANYOF;
    case EConstraintType::OneOf:
        return EOS_EComparisonOp::EOS_CO_ONEOF;
    case EConstraintType::NotOneOf:
        return EOS_EComparisonOp::EOS_CO_NOTONEOF;
    case EConstraintType::Contains:
        return EOS_EComparisonOp::EOS_CO_CONTAINS;
    default:
        // @todo Should this be a check() ?
        return EOS_EComparisonOp::EOS_CO_EQUAL;
    }
}

const FRoomAttribute &FRoomAttributeConstraint::Attribute() const
{
    return this->_Attribute;
}

const FRoomAttributeConstraint::EConstraintType &FRoomAttributeConstraint::ConstraintType() const
{
    return this->_ConstraintType;
}

}

REDPOINT_EOS_CODE_GUARD_END()