// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1877338979, Redpoint::EOS::Tests::RoomSystem::Presence)
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
    CreateRequest.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
        EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
        EPartyConfigurationRoomFeaturePermissionType::Noone,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        TEXT(""),
        TEXT(""),
        TEXT("")));
    auto CreateRoomResult = co_await CreateRoom(RoomSystem, CreateRequest);
    auto CreateErrorCode = CreateRoomResult.Get<0>();
    auto Room = CreateRoomResult.Get<1>();
    if (!Assert->TestTrue(
            TEXT("Expected automation test to be able to create presence room."),
            CreateErrorCode.WasSuccessful()))
    {
        co_return;
    }

    // Make sure not changing the feature doesn't cause an update of it.
    bool bSawPartyConfigurationChange = false;
    auto Handle = RoomSystem->OnRoomUpdated(NAME_Game).AddLambda([&bSawPartyConfigurationChange, Room](
                                                                     const FRoomEventContext &EventContext,
                                                                     const FRoomRef &EventRoom,
                                                                     const FRoomUpdateInfo &EventRoomUpdateInfo) {
        if (EventRoom != Room)
        {
            return;
        }

        if (EventRoomUpdateInfo.FeaturesUpdated.Contains(IPartyConfigurationRoomFeature::GetFeatureName()))
        {
            bSawPartyConfigurationChange = true;
        }
    });
    auto UpdateRequest = FUpdateRoomRequest();
    UpdateRequest.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
        EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
        EPartyConfigurationRoomFeaturePermissionType::Noone,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        TEXT(""),
        TEXT(""),
        TEXT("")));
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
    RoomSystem->OnRoomUpdated(NAME_Game).Remove(Handle);

    // Ensure the event fired *without* the party config feature being marked as changed.
    Assert->TestFalse(TEXT("bSawPartyConfigurationChange should be false!"), bSawPartyConfigurationChange);

    // Make sure changing the feature causes an update of it.
    bSawPartyConfigurationChange = false;
    Handle = RoomSystem->OnRoomUpdated(NAME_Game).AddLambda([&bSawPartyConfigurationChange, Room](
                                                                const FRoomEventContext &EventContext,
                                                                const FRoomRef &EventRoom,
                                                                const FRoomUpdateInfo &EventRoomUpdateInfo) {
        if (EventRoom != Room)
        {
            return;
        }

        if (EventRoomUpdateInfo.FeaturesUpdated.Contains(IPartyConfigurationRoomFeature::GetFeatureName()))
        {
            bSawPartyConfigurationChange = true;
        }
    });
    UpdateRequest = FUpdateRoomRequest();
    UpdateRequest.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
        EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
        EPartyConfigurationRoomFeaturePermissionType::Friends,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        TEXT(""),
        TEXT(""),
        TEXT("")));
    UpdateErrorCode = co_await UpdateRoom(
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
    RoomSystem->OnRoomUpdated(NAME_Game).Remove(Handle);

    // Ensure the event fired with the party config feature changed.
    Assert->TestTrue(TEXT("bSawPartyConfigurationChange should be true!"), bSawPartyConfigurationChange);

    co_return;
}

static auto UpdatePresenceRoom = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.UpdatePresenceRoom.UpdateFires"),
    TestWithPlatforms(TArray<int32>{1}, Test));

}

REDPOINT_EOS_CODE_GUARD_END()
