// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2559867201, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API IRoomMember
{
public:
    IRoomMember() = default;
    IRoomMember(const IRoomMember &) = default;
    IRoomMember &operator=(const IRoomMember &) = default;
    IRoomMember &operator=(IRoomMember &&) = default;
    virtual ~IRoomMember() = default;

    virtual const FRoomUserId &GetUserId() const = 0;
    virtual const TMap<FString, FRoomAttribute> &GetAttributes() const = 0;
};

typedef TSharedRef<IRoomMember> FRoomMemberRef;
typedef TSharedPtr<IRoomMember> FRoomMemberPtr;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(2559867201, Redpoint::EOS::Rooms, IRoomMember)
REDPOINT_EOS_FILE_NS_EXPORT(2559867201, Redpoint::EOS::Rooms, FRoomMemberRef)
REDPOINT_EOS_FILE_NS_EXPORT(2559867201, Redpoint::EOS::Rooms, FRoomMemberPtr)
}

REDPOINT_EOS_CODE_GUARD_END()