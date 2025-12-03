// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2445955054, Redpoint::EOS::Tests::RoomSystem::Presence)
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
    // Create rooms for the other users.
    TArray<FRoomRef> OtherRooms;
    for (int i = 1; i < 3; i++)
    {
        auto &Platform = Platforms[i];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        auto CreateRequest = FCreateRoomRequest(
            ERoomScope::User,
            IdentitySystem->GetUser(0)->GetUserId(),
            FName(TEXT("Lobby")),
            NAME_Game);
        CreateRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        CreateRequest.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
        CreateRequest.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
        CreateRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());

        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, CreateRequest);

        if (!Assert->TestTrue(TEXT("Create room request completed successfully."), ErrorCode.WasSuccessful()) ||
            !Assert->TestTrue(TEXT("Create room request returned a room object."), Room.IsValid()))
        {
            co_return;
        }

        OtherRooms.Add(Room.ToSharedRef());
    }

    {
        auto &Platform = Platforms[0];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        // Try to find the target rooms before we start joining. We can't use JoinRoomReliable because we need to make
        // sure the sequencing is correct.
        auto [SearchErrorCode2, SearchResults2] = co_await SearchRoomsReliable(
            RoomSystem,
            FSearchRoomsRequest(
                IdentitySystem->GetUser(0)->GetUserId(),
                FName(TEXT("Lobby")),
                NAME_Game,
                OtherRooms[0]->GetRoomId()));
        auto [SearchErrorCode3, SearchResults3] = co_await SearchRoomsReliable(
            RoomSystem,
            FSearchRoomsRequest(
                IdentitySystem->GetUser(0)->GetUserId(),
                FName(TEXT("Lobby")),
                NAME_Game,
                OtherRooms[1]->GetRoomId()));
        if (!Assert->TestTrue(TEXT("Search for room 2 completed successfully."), SearchErrorCode2.WasSuccessful()) ||
            !Assert->TestTrue(TEXT("Search for room 3 completed successfully."), SearchErrorCode3.WasSuccessful()) ||
            !Assert->TestTrue(TEXT("Search for room 2 returned non-zero search results."), SearchResults2.Num() > 0) ||
            !Assert->TestTrue(TEXT("Search for room 3 returned non-zero search results."), SearchResults3.Num() > 0))
        {
            co_return;
        }

        // Start create operation, then concurrent join operations. We should end up being in the room that the last
        // join request refers to.
        auto CreateRequest = FCreateRoomRequest(
            ERoomScope::User,
            IdentitySystem->GetUser(0)->GetUserId(),
            FName(TEXT("Lobby")),
            NAME_Game);
        CreateRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        CreateRequest.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
        CreateRequest.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
        CreateRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        auto Task1 = CreateRoom(RoomSystem, CreateRequest);
        auto Task2 = JoinRoom(
            RoomSystem,
            FJoinRoomRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), SearchResults2[0]));
        auto Task3 = JoinRoom(
            RoomSystem,
            FJoinRoomRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), SearchResults3[0]));

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
            Assert->TestEqual(
                TEXT("User should only be in room 3 (2nd compare)."),
                CurrentRooms[0]->GetRoomId()->ToString(),
                OtherRooms[1]->GetRoomId()->ToString());
        }
    }

    co_return;
}

static auto JoinPresenceRoomConcurrent = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.JoinPresenceRoom.Concurrent"),
    TestWithPlatforms(TArray<int32>{1, 1, 1}, Test));

}

REDPOINT_EOS_CODE_GUARD_END()
