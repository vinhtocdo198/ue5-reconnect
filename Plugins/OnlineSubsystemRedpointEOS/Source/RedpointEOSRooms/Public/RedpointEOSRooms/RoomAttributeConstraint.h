// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/LobbySessionsShared/AttributeData.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomAttribute.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(671663452, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::API::LobbySessionsShared;

class REDPOINTEOSROOMS_API FRoomAttributeConstraint
{
public:
    enum class EConstraintType : uint8
    {
        Equal,
        NotEqual,
        GreaterThan,
        GreaterThanOrEqual,
        LessThan,
        LessThanOrEqual,
        Distance,
        AnyOf,
        NotAnyOf,
        OneOf,
        NotOneOf,
        Contains,
    };

private:
    FRoomAttribute _Attribute;
    EConstraintType _ConstraintType;

public:
    FRoomAttributeConstraint(const FRoomAttribute &Attribute, EConstraintType ConstraintType);

    EOS_EComparisonOp GetComparisonOp() const;

    const FRoomAttribute &Attribute() const;
    const EConstraintType &ConstraintType() const;
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(671663452, Redpoint::EOS::Rooms, FRoomAttributeConstraint)
}

REDPOINT_EOS_CODE_GUARD_END()