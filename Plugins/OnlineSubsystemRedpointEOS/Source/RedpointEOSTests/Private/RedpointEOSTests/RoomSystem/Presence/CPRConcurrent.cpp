// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2552246314, Redpoint::EOS::Tests::RoomSystem::Presence)
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

    // Define the create operation. This test enables presence and party type to test exclusivity keys that come from
    // features.
    auto CreateRequest =
        FCreateRoomRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), FName(TEXT("Lobby")), NAME_Game);
    CreateRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
    CreateRequest.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));

    // Start multiple create operations concurrently. The subsequent ones should be deferred and complete without
    // failure.
    auto Task1 = CreateRoom(RoomSystem, CreateRequest);
    auto Task2 = CreateRoom(RoomSystem, CreateRequest);
    auto Task3 = CreateRoom(RoomSystem, CreateRequest);

    // Check the results.
    auto [ErrorCode1, Room1] = co_await Task1;
    auto [ErrorCode2, Room2] = co_await Task2;
    auto [ErrorCode3, Room3] = co_await Task3;

    Assert->TestTrue(TEXT("Request 1 completed successfully."), ErrorCode1.WasSuccessful());
    Assert->TestTrue(TEXT("Request 2 completed successfully."), ErrorCode2.WasSuccessful());
    Assert->TestTrue(TEXT("Request 3 completed successfully."), ErrorCode3.WasSuccessful());

    Assert->TestTrue(TEXT("Request 1 returned a room object."), Room1.IsValid());
    Assert->TestTrue(TEXT("Request 2 returned a room object."), Room2.IsValid());
    Assert->TestTrue(TEXT("Request 3 returned a room object."), Room3.IsValid());

    // Ensure the user is only in room 3.
    auto CurrentRooms = RoomSystem->GetCurrentRooms(
        FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), NAME_Game));
    if (Assert->TestEqual(TEXT("Number of rooms user is currently joined to."), CurrentRooms.Num(), 1))
    {
        Assert->TestEqual(
            TEXT("User should only be in room 3."),
            CurrentRooms[0]->GetRoomId()->ToString(),
            Room3->GetRoomId()->ToString());
    }

    co_return;
}

static auto CreatePresenceRoomConcurrent = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.CreatePresenceRoom.Concurrent"),
    TestWithPlatforms(TArray<int32>{1}, Test));

}

REDPOINT_EOS_CODE_GUARD_END()
