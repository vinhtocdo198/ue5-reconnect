// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Requests/GetCurrentRoomsRequest.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1758651349, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms::Requests;

FGetCurrentRoomsRequest::FGetCurrentRoomsRequest(
    ERoomScope InRoomScope,
    FName InRoomNamespace,
    const TMap<FString, FRoomAttributeConstraint> &InAttributeConstraints)
    : RoomScope(InRoomScope)
    , LocalUserId()
    , RoomNamespace(InRoomNamespace)
    , AttributeConstraints(InAttributeConstraints)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Requests);

    checkf(
        InRoomScope == ERoomScope::Global,
        TEXT("Single argument FGetCurrentRoomsRequest should have ERoomScope::Global."));
}

FGetCurrentRoomsRequest::FGetCurrentRoomsRequest(
    ERoomScope InRoomScope,
    FRoomUserId InLocalUserId,
    FName InRoomNamespace,
    const TMap<FString, FRoomAttributeConstraint> &InAttributeConstraints)
    : RoomScope(InRoomScope)
    , LocalUserId(InLocalUserId)
    , RoomNamespace(InRoomNamespace)
    , AttributeConstraints(InAttributeConstraints)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()