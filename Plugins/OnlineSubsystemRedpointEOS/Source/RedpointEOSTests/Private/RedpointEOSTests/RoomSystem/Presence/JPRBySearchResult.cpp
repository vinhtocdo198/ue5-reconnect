// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2204280420, Redpoint::EOS::Tests::RoomSystem::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Tests::Async;
using namespace ::Redpoint::EOS::Tests::RoomSystem::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Rooms::Features;

static TTask<void>::With<ETaskBinding::Static> TestBySearchResult(
    TSharedRef<FAutomationTestBase> Assert,
    TArray<FPlatformHandle> Platforms)
{
    // Create the presence room on the first user's system.
    FRoomIdPtr TargetRoomId;
    FString TargetGuid = FGuid::NewGuid().ToString();
    {
        auto &Platform = Platforms[0];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        auto CreateRequest = FCreateRoomRequest(
            ERoomScope::User,
            IdentitySystem->GetUser(0)->GetUserId(),
            FName(TEXT("Lobby")),
            NAME_Game);
        CreateRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        CreateRequest.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
        CreateRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        CreateRequest.Attributes.Add(TEXT("SearchGuid"), TargetGuid);
        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, CreateRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to create presence room for first user."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }

        // Set the target room ID to be the first room.
        TargetRoomId = Room->GetRoomId();
    }

    // Create a presence room on the second user's system (the one they will be in when they attempt to join the new
    // one).
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        auto CreateRequest = FCreateRoomRequest(
            ERoomScope::User,
            IdentitySystem->GetUser(0)->GetUserId(),
            FName(TEXT("Lobby")),
            NAME_Game);
        CreateRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        CreateRequest.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
        CreateRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, CreateRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to create presence room for second user."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }
    }

    // Perform a durable search for the first room as the second user, then join it. They should automatically leave the
    // presence room they are currently in.
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        auto SearchRequest = FSearchRoomsRequest(
            IdentitySystem->GetUser(0)->GetUserId(),
            FName(TEXT("Lobby")),
            NAME_Game,
            TMap<FString, FRoomAttributeConstraint>{
                {TEXT("SearchGuid"),
                 FRoomAttributeConstraint(
                     FRoomAttribute(TargetGuid),
                     FRoomAttributeConstraint::EConstraintType::Equal)}});
        SearchRequest.bDurable = true;

        auto [SearchErrorCode, SearchResults] = co_await SearchRooms(RoomSystem, SearchRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to search for second user's room."),
                SearchErrorCode.WasSuccessful()))
        {
            co_return;
        }
        if (!Assert->TestEqual(
                TEXT("Expected automation test to be able to find second user's room via search."),
                SearchResults.Num(),
                1))
        {
            co_return;
        }

        auto JoinRequest =
            FJoinRoomRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), SearchResults[0]);
        JoinRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        auto [ErrorCode, Room] = co_await JoinRoomReliable(Assert, RoomSystem, JoinRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to join first user's presence room as second user."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }

        // Ensure the user is only joined to a single room.
        auto CurrentRooms = RoomSystem->GetCurrentRooms(
            FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), NAME_Game));
        if (Assert->TestEqual(TEXT("Number of rooms second user is currently joined to."), CurrentRooms.Num(), 1))
        {
            // Ensure that the room the user is joined to is room 2.
            Assert->TestEqual(
                TEXT("Second user should only be in the target room."),
                CurrentRooms[0]->GetRoomId()->ToString(),
                TargetRoomId->ToString());
        }
    }

    co_return;
}

static auto JoinPresenceRoomBySearchResult = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.JoinPresenceRoom.BySearchResult"),
    TestWithPlatforms(TArray<int32>{1, 1}, TestBySearchResult));

}

REDPOINT_EOS_CODE_GUARD_END()