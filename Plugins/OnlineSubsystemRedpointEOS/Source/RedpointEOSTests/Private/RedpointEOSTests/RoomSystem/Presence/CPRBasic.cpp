// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1096874321, Redpoint::EOS::Tests::RoomSystem::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Tests::Async;
using namespace ::Redpoint::EOS::Tests::RoomSystem::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Rooms::Features;

static TTask<void>::With<ETaskBinding::Static> Test(
    TSharedRef<FAutomationTestBase> Assert,
    TArray<FPlatformHandle> Platforms)
{
    auto &Platform = Platforms[0];

    auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
    auto RoomSystem = Platform->GetSystem<IRoomSystem>();

    // Create the first presence-enabled room.
    auto CreateRequest1 =
        FCreateRoomRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), FName(TEXT("Lobby")), NAME_Game);
    CreateRequest1.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
    CreateRequest1.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
    auto [ErrorCode1, Room1] = co_await CreateRoom(RoomSystem, CreateRequest1);
    if (!Assert->TestTrue(
            TEXT("Expected automation test to be able to create first presence room."),
            ErrorCode1.WasSuccessful()))
    {
        co_return;
    }

    // Create the second presence-enabled room.
    auto CreateRequest2 =
        FCreateRoomRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), FName(TEXT("Lobby")), NAME_Game);
    CreateRequest2.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
    CreateRequest1.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
    auto [ErrorCode2, Room2] = co_await CreateRoom(RoomSystem, CreateRequest2);
    if (!Assert->TestTrue(
            TEXT("Expected automation test to be able to create first presence room."),
            ErrorCode2.WasSuccessful()))
    {
        co_return;
    }

    // Ensure the user is only joined to a single room.
    auto CurrentRooms = RoomSystem->GetCurrentRooms(
        FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), NAME_Game));
    if (Assert->TestEqual(TEXT("Number of rooms user is currently joined to."), CurrentRooms.Num(), 1))
    {
        // Ensure that the room the user is joined to is room 2.
        Assert->TestEqual(
            TEXT("User should only be in room 2."),
            CurrentRooms[0]->GetRoomId()->ToString(),
            Room2->GetRoomId()->ToString());
    }

    co_return;
}

static auto CreatePresenceRoom = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.CreatePresenceRoom.Basic"),
    TestWithPlatforms(TArray<int32>{1}, Test));

}

REDPOINT_EOS_CODE_GUARD_END()
