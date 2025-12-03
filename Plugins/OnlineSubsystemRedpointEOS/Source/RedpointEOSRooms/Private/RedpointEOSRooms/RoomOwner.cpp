// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomOwner.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()
namespace REDPOINT_EOS_FILE_NS_ID(1756786144, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

FRoomOwner::FRoomOwner(const FRoomUserId &InUserId)
    : bIsDedicatedServer(false)
    , UserId(InUserId)
    , DedicatedServerClientId()
{
}

FRoomOwner::FRoomOwner(const FString &InDedicatedServerClientId)
    : bIsDedicatedServer(true)
    , UserId()
    , DedicatedServerClientId(InDedicatedServerClientId)
{
}

bool FRoomOwner::IsDedicatedServer() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->bIsDedicatedServer;
}

FRoomUserId FRoomOwner::GetUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->UserId;
}

FString FRoomOwner::GetDedicatedServerClientId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->DedicatedServerClientId;
}

}
REDPOINT_EOS_CODE_GUARD_END()