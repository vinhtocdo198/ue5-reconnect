// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSTests/Async/TestWithPlatforms.h"
#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3864172765, Redpoint::EOS::Tests::RoomSystem::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Tests::Async;
using namespace ::Redpoint::EOS::Tests::RoomSystem::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Rooms::Features;

static TTask<void>::With<ETaskBinding::Static> TestByInvite(
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
        CreateRequest.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
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
        CreateRequest.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
        auto [ErrorCode, Room] = co_await CreateRoom(RoomSystem, CreateRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to create presence room for second user."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }
    }

    // Register a handler on the second user to store incoming invites.
    FRoomInvitePtr ReceivedRoomInvite;
    FDelegateHandle RoomInviteDelegateHandle;
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        RoomInviteDelegateHandle = RoomSystem->OnRoomInviteReceived(NAME_Game).AddLambda(
            [&ReceivedRoomInvite,
             TargetRoomId](const FRoomEventContext &EventContext, const FRoomInviteRef &RoomInvite) {
                if (RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->IsEqual(TargetRoomId.ToSharedRef()))
                {
                    ReceivedRoomInvite = RoomInvite;
                }
            });
    }

    // Send an invite from the first user to the second user.
    {
        auto &SenderPlatform = Platforms[0];

        auto SenderIdentitySystem = SenderPlatform->GetSystem<IIdentitySystem>();
        auto SenderRoomSystem = SenderPlatform->GetSystem<IRoomSystem>();

        auto &ReceiverPlatform = Platforms[1];

        auto ReceiverIdentitySystem = ReceiverPlatform->GetSystem<IIdentitySystem>();
        auto ReceiverRoomSystem = ReceiverPlatform->GetSystem<IRoomSystem>();

        FSendRoomInviteRequest SendInviteRequest;
        SendInviteRequest.TargetUser = ReceiverIdentitySystem->GetUser(0)->GetUserId();

        auto ErrorCode = co_await SendRoomInvite(
            SenderRoomSystem,
            SenderRoomSystem->GetCurrentRooms(
                FGetCurrentRoomsRequest(ERoomScope::User, SenderIdentitySystem->GetUser(0)->GetUserId(), NAME_Game))[0],
            SendInviteRequest);
        if (!Assert->TestTrue(
                TEXT("Expected automation test to be able to send room invite to second user."),
                ErrorCode.WasSuccessful()))
        {
            co_return;
        }
    }

    // Wait for the invite to be received by the second user.
    {
        int Attempts = 0;
        while (!ReceivedRoomInvite.IsValid() && Attempts < 30)
        {
            co_await Delay(1.0f);
            Attempts++;
        }
        if (!Assert->TestTrue(
                TEXT("Expected second user in automation test to receive invite within 30 second timeout."),
                ReceivedRoomInvite.IsValid()))
        {
            co_return;
        }
    }

    // Join the room as the second user using the room invite. They should automatically leave the presence room they
    // are currently in.
    {
        auto &Platform = Platforms[1];

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();
        auto RoomSystem = Platform->GetSystem<IRoomSystem>();

        auto JoinRequest = FJoinRoomRequest(ERoomScope::User, ReceivedRoomInvite.ToSharedRef());
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

static auto JoinPresenceRoomByInvite = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.RoomSystem.Presence.JoinPresenceRoom.ByInvite"),
    TestWithPlatforms(TArray<int32>{1, 1}, TestByInvite));

}

REDPOINT_EOS_CODE_GUARD_END()