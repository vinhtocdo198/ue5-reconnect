// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Requests/CreateRoomRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2655719296, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms::Requests;

FCreateRoomRequest::FCreateRoomRequest(
    ERoomScope InRoomScope,
    FRoomUserId InLocalUser,
    FName InRoomProvider,
    FName InRoomNamespace)
    : RoomScope(InRoomScope)
    , LocalUser(InLocalUser)
    , RoomProvider(InRoomProvider)
    , RoomNamespace(InRoomNamespace)
    , MaxMemberCount(4)
    , Attributes()
    , MemberAttributes()
{
}

FCreateRoomRequest::FCreateRoomRequest(
    ERoomScope InRoomScope,
    FRoomUserId InLocalUser,
    FName InRoomProvider,
    FName InRoomNamespace,
    uint32 InMaxMemberCount)
    : RoomScope(InRoomScope)
    , LocalUser(InLocalUser)
    , RoomProvider(InRoomProvider)
    , RoomNamespace(InRoomNamespace)
    , MaxMemberCount(InMaxMemberCount)
    , Attributes()
    , MemberAttributes()
{
}

}

REDPOINT_EOS_CODE_GUARD_END()