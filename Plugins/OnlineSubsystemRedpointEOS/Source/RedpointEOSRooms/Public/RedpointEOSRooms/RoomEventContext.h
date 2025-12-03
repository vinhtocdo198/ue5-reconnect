// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/Optional.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/RoomScope.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(43897010, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

struct REDPOINTEOSROOMS_API FRoomEventContext
{
public:
    /**
     * The scope of the room that this event is being raised for.
     */
    ERoomScope RoomScope;

    /**
     * If this room is scoped to a local user, this is the local user the event is firing for.
     */
    FRoomUserId LocalUserId;

private:
    FRoomEventContext(ERoomScope InRoomScope, FRoomUserId InLocalUserId)
        : RoomScope(InRoomScope)
        , LocalUserId(InLocalUserId){};

public:
    FRoomEventContext(const FRoomEventContext &) = default;
    FRoomEventContext(FRoomEventContext &&) = default;
    ~FRoomEventContext() = default;

    template <ERoomScope = ERoomScope::Global> static FRoomEventContext Create()
    {
        return FRoomEventContext(ERoomScope::Global, FRoomUserId());
    }
    template <ERoomScope = ERoomScope::User> static FRoomEventContext Create(FRoomUserId InLocalUserId)
    {
        return FRoomEventContext(ERoomScope::User, InLocalUserId);
    }

    FString ToString() const
    {
        if (this->RoomScope == ERoomScope::Global)
        {
            return TEXT("global");
        }
        else
        {
            return FString::Printf(TEXT("user:%s"), *::Redpoint::EOS::Core::Id::GetUserIdString(this->LocalUserId));
        }
    }
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(43897010, Redpoint::EOS::Rooms, FRoomEventContext)
}

REDPOINT_EOS_CODE_GUARD_END()