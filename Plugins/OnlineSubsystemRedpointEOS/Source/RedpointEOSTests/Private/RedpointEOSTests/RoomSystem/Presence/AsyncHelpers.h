// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;

TTask<TTuple<FError, FRoomPtr>> CreateRoom(TSharedRef<IRoomSystem> RoomSystem, FCreateRoomRequest CreateRoomRequest);

TTask<FError> UpdateRoom(TSharedRef<IRoomSystem> RoomSystem, FRoomRef Room, FUpdateRoomRequest UpdateRoomRequest);

TTask<TTuple<FError, FRoomPtr>> JoinRoom(TSharedRef<IRoomSystem> RoomSystem, FJoinRoomRequest JoinRoomRequest);

TTask<FError> LeaveRoom(TSharedRef<IRoomSystem> RoomSystem, FRoomRef Room, FLeaveRoomRequest LeaveRoomRequest);

TTask<void> Delay(float Seconds);

TTask<TTuple<FError, FRoomPtr>> JoinRoomReliable(
    TSharedRef<FAutomationTestBase> Assert,
    TSharedRef<IRoomSystem> RoomSystem,
    FJoinRoomRequest JoinRoomRequest);

TTask<TTuple<FError, TArray<FRoomSearchResultRef>>> SearchRooms(
    TSharedRef<IRoomSystem> RoomSystem,
    FSearchRoomsRequest SearchRoomsRequest);

TTask<TTuple<FError, TArray<FRoomSearchResultRef>>> SearchRoomsReliable(
    TSharedRef<IRoomSystem> RoomSystem,
    FSearchRoomsRequest SearchRoomsRequest);

TTask<FError> SendRoomInvite(
    TSharedRef<IRoomSystem> RoomSystem,
    FRoomRef Room,
    FSendRoomInviteRequest SendInviteRequest);

}

namespace Redpoint::EOS::Tests::RoomSystem::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, CreateRoom);
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, UpdateRoom);
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, JoinRoom);
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, LeaveRoom);
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, Delay);
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, JoinRoomReliable);
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, SearchRooms);
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, SearchRoomsReliable);
REDPOINT_EOS_FILE_NS_EXPORT(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence, SendRoomInvite);
}

REDPOINT_EOS_CODE_GUARD_END()