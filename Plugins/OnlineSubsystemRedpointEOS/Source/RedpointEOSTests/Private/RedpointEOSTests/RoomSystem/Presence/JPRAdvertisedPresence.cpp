// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(188910197, Redpoint::EOS::Tests::RoomSystem::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Tests::Async;
using namespace ::Redpoint::EOS::Tests::RoomSystem::Presence;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Rooms::Features;

static TTask<void>::With<ETaskBinding::Static> TestAdvertisedPresence(
    TSharedRef<FAutomationTestBase> Assert,
    TArray<FPlatformHandle> Platforms)
{
    // Create the presence room on the first user's system.
    FRoomIdPtr TargetRoomId1;
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
        CreateRequest.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, CreateRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to create presence room for first user."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }
        TargetRoomId1 = Room->GetRoomId();
    }

    // Create a presence room on the second user's system (the one they will be in when they attempt to join the new
    // one).
    FRoomIdPtr TargetRoomId2;
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
        CreateRequest.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, CreateRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to create presence room for second user."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }
        TargetRoomId2 = Room->GetRoomId();
    }

    // Confirm that the second user's advertised presence from their perspective is advertising their current room.
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();
        auto PresenceSystem = Platform->GetSystem<IPresenceSystem>();

        auto CurrentPresence = PresenceSystem->GetPresence(
            IdentitySystem->GetUser(0)->GetUserId(),
            IdentitySystem->GetUser(0)->GetUserId());
        if (!Assert->TestTrue(
                TEXT("Expected to be able to immediately get the presence status of the local user."),
                CurrentPresence.IsSet()))
        {
            co_return;
        }

        if (Assert->TestTrue(TEXT("Second user is advertising session."), CurrentPresence->AdvertisedSession.IsSet()))
        {
            Assert->TestTrue(
                TEXT("Second user is advertising desired room as session."),
                CurrentPresence->AdvertisedSession->RoomId->IsEqual(TargetRoomId2.ToSharedRef()));
            Assert->TestTrue(
                TEXT("Second user is advertising desired room as joinable."),
                CurrentPresence->AdvertisedSession->bJoinable);
        }
    }

    // Join the target presence room on the second user's system. They should automatically leave the presence room they
    // are currently in.
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        auto JoinRequest =
            FJoinRoomRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), TargetRoomId1.ToSharedRef());
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
                TargetRoomId1->ToString());
        }
    }

    // Confirm that the second user's advertised presence from their perspective is now advertising the first room.
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();
        auto PresenceSystem = Platform->GetSystem<IPresenceSystem>();

        auto CurrentPresence = PresenceSystem->GetPresence(
            IdentitySystem->GetUser(0)->GetUserId(),
            IdentitySystem->GetUser(0)->GetUserId());
        if (!Assert->TestTrue(
                TEXT("Expected to be able to immediately get the presence status of the local user."),
                CurrentPresence.IsSet()))
        {
            co_return;
        }

        if (Assert->TestTrue(TEXT("Second user is advertising session."), CurrentPresence->AdvertisedSession.IsSet()))
        {
            Assert->TestTrue(
                TEXT("Second user is advertising desired room as session."),
                CurrentPresence->AdvertisedSession->RoomId->IsEqual(TargetRoomId1.ToSharedRef()));
            Assert->TestTrue(
                TEXT("Second user is advertising desired room as joinable."),
                CurrentPresence->AdvertisedSession->bJoinable);
        }
    }

    // Leave the room as the second user.
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        auto Room = RoomSystem->GetCurrentRooms(
            FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), NAME_Game))[0];

        auto LeaveRequest = FLeaveRoomRequest();
        auto ErrorCode = co_await LeaveRoom(RoomSystem, Room, LeaveRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to leave presence room on second user."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }

        // Ensure the user is no longer in any rooms.
        auto CurrentRooms = RoomSystem->GetCurrentRooms(
            FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), NAME_Game));
        Assert->TestEqual(TEXT("Number of rooms second user is currently joined to."), CurrentRooms.Num(), 0);
    }

    // Confirm that the second user's advertised presence from their perspective is no longer advertising any room.
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();
        auto PresenceSystem = Platform->GetSystem<IPresenceSystem>();

        auto CurrentPresence = PresenceSystem->GetPresence(
            IdentitySystem->GetUser(0)->GetUserId(),
            IdentitySystem->GetUser(0)->GetUserId());
        if (!Assert->TestTrue(
                TEXT("Expected to be able to immediately get the presence status of the local user."),
                CurrentPresence.IsSet()))
        {
            co_return;
        }

        Assert->TestTrue(TEXT("Second user is advertising session."), !CurrentPresence->AdvertisedSession.IsSet());
    }

    co_return;
}

static auto JoinPresenceRoomAdvertisedPresence = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.JoinPresenceRoom.AdvertisedPresence"),
    TestWithPlatforms(TArray<int32>{1, 1}, TestAdvertisedPresence));

}

REDPOINT_EOS_CODE_GUARD_END()