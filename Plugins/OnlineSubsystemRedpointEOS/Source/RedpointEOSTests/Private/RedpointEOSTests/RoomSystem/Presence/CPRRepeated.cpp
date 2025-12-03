// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1671863499, Redpoint::EOS::Tests::RoomSystem::Presence)
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

    // Repeatedly create presence-enabled rooms.
    FRoomPtr LatestRoom;
    for (int32 i = 0; i <= 6 /* Must have (i % 3) == 0 on last iteration. */; i++)
    {
        auto CreateRequest = FCreateRoomRequest(
            ERoomScope::User,
            IdentitySystem->GetUser(0)->GetUserId(),
            FName(TEXT("Lobby")),
            NAME_Game);
        if ((i % 3) == 0 || (i % 3) == 1)
        {
            CreateRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        }
        if ((i % 3) == 0 || (i % 3) == 2)
        {
            CreateRequest.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
        }
        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, CreateRequest);
        if (!Assert->TestTrue(
                FString::Printf(
                    TEXT("Expected automation test to be able to create presence room on attempt %d: %s"),
                    i + 1,
                    *ErrorCode.ToLogString()),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }
        LatestRoom = Room;
    }

    // Ensure the user is only joined to a single room.
    auto CurrentRooms = RoomSystem->GetCurrentRooms(
        FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), NAME_Game));
    if (Assert->TestEqual(TEXT("Number of rooms user is currently joined to."), CurrentRooms.Num(), 1))
    {
        // Ensure that the room the user is joined to is room 2.
        Assert->TestEqual(
            TEXT("User should only be in latest room."),
            CurrentRooms[0]->GetRoomId()->ToString(),
            LatestRoom->GetRoomId()->ToString());
    }

    co_return;
}

static auto CreatePresenceRoomRepeated = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.CreatePresenceRoom.Repeated"),
    TestWithPlatforms(TArray<int32>{1}, Test));

}

REDPOINT_EOS_CODE_GUARD_END()
