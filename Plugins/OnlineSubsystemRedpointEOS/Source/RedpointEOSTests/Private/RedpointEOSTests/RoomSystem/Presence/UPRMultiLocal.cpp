// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(367280999, Redpoint::EOS::Tests::RoomSystem::Presence)
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

    FRoomPtr Room1, Room2;

    auto EnsureInSingleRoom = [Assert, IdentitySystem, RoomSystem](int32 UserSlot, FRoomPtr Room) {
        auto CurrentRooms = RoomSystem->GetCurrentRooms(
            FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(UserSlot)->GetUserId(), NAME_Game));
        if (!Assert->TestEqual(TEXT("Number of rooms user is currently joined to."), CurrentRooms.Num(), 1))
        {
            return false;
        }
        if (!Assert->TestEqual(
                TEXT("User should still be in the room."),
                CurrentRooms[0]->GetRoomId()->ToString(),
                Room->GetRoomId()->ToString()))
        {
            return false;
        }
        return true;
    };

    // User 1: Create the presence-enabled room.
    {
        auto Request = FCreateRoomRequest(
            ERoomScope::User,
            IdentitySystem->GetUser(0)->GetUserId(),
            FName(TEXT("Lobby")),
            NAME_Game,
            4);
        Request.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>(TEXT("TestBucket")));
        Request.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
            EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            TEXT(""),
            TEXT(""),
            TEXT("")));
        Request.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
        Request.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
        Request.AddFeatureRequest(MakeShared<FPublicMemberListRoomFeatureRequest>());
        Request.AddFeatureRequest(MakeShared<FOwnerMigrationRoomFeatureRequest>());
        Request.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, Request);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to create presence room."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }
        Room1 = Room;
    }

    // User 1: Ensure the current rooms match the expectations.
    if (!EnsureInSingleRoom(0, Room1))
    {
        co_return;
    }

    // User 2: Create the presence-enabled room.
    {
        auto Request = FCreateRoomRequest(
            ERoomScope::User,
            IdentitySystem->GetUser(1)->GetUserId(),
            FName(TEXT("Lobby")),
            NAME_Game,
            4);
        Request.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>(TEXT("TestBucket")));
        Request.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
            EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            TEXT(""),
            TEXT(""),
            TEXT("")));
        // @note: We don't add PartyType = 1000 on the second room so that when the second user's presence room is
        // updated, we ensure the room database doesn't acquire the PartyType:1000 exclusive key as a transitive
        // dependency (because both the user's rooms have the Presence exclusive key).
        //
        // Request.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
        Request.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
        Request.AddFeatureRequest(MakeShared<FPublicMemberListRoomFeatureRequest>());
        Request.AddFeatureRequest(MakeShared<FOwnerMigrationRoomFeatureRequest>());
        Request.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, Request);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to create presence room."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }
        Room2 = Room;
    }

    // User 1: Ensure the current rooms match the expectations.
    if (!EnsureInSingleRoom(0, Room1))
    {
        co_return;
    }

    // User 2: Ensure the current rooms match the expectations.
    if (!EnsureInSingleRoom(1, Room2))
    {
        co_return;
    }

    // User 1: Update the presence-enabled room.
    {
        auto Request = FUpdateRoomRequest();
        Request.MaxMemberCount = 4;
        Request.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
            EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            TEXT(""),
            TEXT(""),
            TEXT("")));
        Request.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        Request.UpsertAttributes.Add(TEXT("MyAttribute"), FRoomAttribute(TEXT("HelloWorld")));
        auto UpdateErrorCode = co_await UpdateRoom(RoomSystem, Room1.ToSharedRef(), Request);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to update presence room."),
                UpdateErrorCode.WasSuccessful()))
        {
            co_return;
        }
    }

    // User 1: Ensure the current rooms match the expectations.
    if (!EnsureInSingleRoom(0, Room1))
    {
        co_return;
    }

    // User 2: Ensure the current rooms match the expectations.
    if (!EnsureInSingleRoom(1, Room2))
    {
        co_return;
    }

    // User 2: Update the presence-enabled room.
    {
        auto Request = FUpdateRoomRequest();
        Request.MaxMemberCount = 4;
        Request.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
            EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            TEXT(""),
            TEXT(""),
            TEXT("")));
        Request.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        Request.UpsertAttributes.Add(TEXT("MyAttribute"), FRoomAttribute(TEXT("HelloWorld")));
        auto UpdateErrorCode = co_await UpdateRoom(RoomSystem, Room2.ToSharedRef(), Request);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to update presence room."),
                UpdateErrorCode.WasSuccessful()))
        {
            co_return;
        }
    }

    // User 1: Ensure the current rooms match the expectations.
    if (!EnsureInSingleRoom(0, Room1))
    {
        co_return;
    }

    // User 2: Ensure the current rooms match the expectations.
    if (!EnsureInSingleRoom(1, Room2))
    {
        co_return;
    }

    co_return;
}

static auto UpdatePresenceRoomMultiLocal = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.UpdatePresenceRoom.MultiLocal"),
    TestWithPlatforms(TArray<int32>{2}, Test));

}

REDPOINT_EOS_CODE_GUARD_END()
