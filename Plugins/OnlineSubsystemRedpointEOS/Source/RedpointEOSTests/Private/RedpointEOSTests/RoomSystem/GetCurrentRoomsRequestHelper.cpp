// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"

namespace REDPOINT_EOS_FILE_NS_ID(2073920046, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;

FGetCurrentRoomsRequest GetCurrentRoomsRequest(const FRoomUserId &RoomUserId)
{
    return FGetCurrentRoomsRequest(
        ERoomScope::User,
        RoomUserId,
        NAME_AutomationTests,
        TMap<FString, FRoomAttributeConstraint>());
}

}