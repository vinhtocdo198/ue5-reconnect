// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1756786144, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FRoomOwner
{
private:
    bool bIsDedicatedServer;
    FRoomUserId UserId;
    FString DedicatedServerClientId;

public:
    FRoomOwner(const FRoomUserId &InUserId);
    FRoomOwner(const FString &InDedicatedServerClientId);
    UE_NONCOPYABLE(FRoomOwner);
    ~FRoomOwner() = default;

    bool IsDedicatedServer() const;
    FRoomUserId GetUserId() const;
    FString GetDedicatedServerClientId() const;
};

typedef TSharedRef<const FRoomOwner> FRoomOwnerRef;
typedef TSharedPtr<const FRoomOwner> FRoomOwnerPtr;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(1756786144, Redpoint::EOS::Rooms, FRoomOwner)
REDPOINT_EOS_FILE_NS_EXPORT(1756786144, Redpoint::EOS::Rooms, FRoomOwnerRef)
REDPOINT_EOS_FILE_NS_EXPORT(1756786144, Redpoint::EOS::Rooms, FRoomOwnerPtr)
}

REDPOINT_EOS_CODE_GUARD_END()