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

namespace REDPOINT_EOS_FILE_NS_ID(1517109380, Redpoint::EOS::Tests::RoomSystem::Presence)
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

    // Create the presence-enabled room.
    auto CreateRequest = FCreateRoomRequest(
        ERoomScope::User,
        IdentitySystem->GetUser(0)->GetUserId(),
        FName(TEXT("Lobby")),
        NAME_Game,
        4);
    CreateRequest.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>(TEXT("TestBucket")));
    CreateRequest.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
        EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        TEXT(""),
        TEXT(""),
        TEXT("")));
    CreateRequest.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(1000));
    CreateRequest.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
    CreateRequest.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
    CreateRequest.AddFeatureRequest(MakeShared<FPublicMemberListRoomFeatureRequest>());
    CreateRequest.AddFeatureRequest(MakeShared<FOwnerMigrationRoomFeatureRequest>());
    CreateRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
    CreateRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
    auto [CreateErrorCode, Room] = co_await CreateRoom(RoomSystem, CreateRequest);
    if (!Assert->TestTrue(
            TEXT("Expected automation test to be able to create presence room."),
            CreateErrorCode.WasSuccessful()))
    {
        co_return;
    }

    // Update the presence-enabled room.
    auto UpdateRequest = FUpdateRoomRequest();
    UpdateRequest.MaxMemberCount = 4;
    UpdateRequest.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
        EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        TEXT(""),
        TEXT(""),
        TEXT("")));
    UpdateRequest.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
    UpdateRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
    UpdateRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
    UpdateRequest.UpsertAttributes.Add(TEXT("MyAttribute"), FRoomAttribute(TEXT("HelloWorld")));
    auto UpdateErrorCode = co_await UpdateRoom(
        RoomSystem,
        RoomSystem->GetCurrentRooms(
            FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), NAME_Game))[0],
        UpdateRequest);
    if (!Assert->TestTrue(
            TEXT("Expected automation test to be able to update presence room."),
            UpdateErrorCode.WasSuccessful()))
    {
        co_return;
    }

    // Ensure the user is still joined to the room.
    auto CurrentRooms = RoomSystem->GetCurrentRooms(
        FGetCurrentRoomsRequest(ERoomScope::User, IdentitySystem->GetUser(0)->GetUserId(), NAME_Game));
    if (Assert->TestEqual(TEXT("Number of rooms user is currently joined to."), CurrentRooms.Num(), 1))
    {
        // Ensure that the room the user is joined to is room 2.
        Assert->TestEqual(
            TEXT("User should still be in the room."),
            CurrentRooms[0]->GetRoomId()->ToString(),
            Room->GetRoomId()->ToString());
    }

    co_return;
}

static auto UpdatePresenceRoom = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.UpdatePresenceRoom.Basic"),
    TestWithPlatforms(TArray<int32>{1}, Test));

}

REDPOINT_EOS_CODE_GUARD_END()
