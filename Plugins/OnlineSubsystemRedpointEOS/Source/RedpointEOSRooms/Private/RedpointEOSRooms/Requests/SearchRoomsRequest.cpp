// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Requests/SearchRoomsRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(86934985, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms::Requests;

FSearchRoomsRequest::FSearchRoomsRequest(
    const FRoomUserId &InLocalUser,
    const FName &InRoomProvider,
    const FName &InRoomNamespace,
    const FRoomIdRef &InRequireRoomId)
    : LocalUser(InLocalUser)
    , RoomProvider(InRoomProvider)
    , RoomNamespace(InRoomNamespace)
    , MaxResultCount()
    , RequireRoomId(InRequireRoomId)
    , RequireRoomMemberId()
    , AttributeConstraints()
    , bDurable(true)
{
}

FSearchRoomsRequest::FSearchRoomsRequest(
    const FRoomUserId &InLocalUser,
    const FName &InRoomProvider,
    const FName &InRoomNamespace,
    const FRoomUserId &InRequireRoomMemberId)
    : LocalUser(InLocalUser)
    , RoomProvider(InRoomProvider)
    , RoomNamespace(InRoomNamespace)
    , MaxResultCount()
    , RequireRoomId()
    , RequireRoomMemberId(InRequireRoomMemberId)
    , AttributeConstraints()
    , bDurable(true)
{
}

FSearchRoomsRequest::FSearchRoomsRequest(
    const FRoomUserId &InLocalUser,
    const FName &InRoomProvider,
    const FName &InRoomNamespace,
    const TMap<FString, FRoomAttributeConstraint> &InAttributeConstraints)
    : LocalUser(InLocalUser)
    , RoomProvider(InRoomProvider)
    , RoomNamespace(InRoomNamespace)
    , MaxResultCount()
    , RequireRoomId()
    , RequireRoomMemberId()
    , AttributeConstraints(InAttributeConstraints)
    , bDurable(true)
{
}
}

REDPOINT_EOS_CODE_GUARD_END()