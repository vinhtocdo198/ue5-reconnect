// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/CreateRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsurePublicMemberListChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsureRoomMembersChainedTask.h"
#include "RedpointEOSTests/RoomSystem/JoinRoomViaSearchChainedTask.h"
#include "RedpointEOSTests/RoomSystem/LeaveRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/PromoteRoomMemberChainedTask.h"
#include "RedpointEOSTests/RoomSystem/UpdateRoomChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(253764966, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_MemoryLeakTest,
    "Redpoint.EOS.RoomSystem.MemoryLeakTest",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter | EAutomationTestFlags::RequiresUser);

void FOnlineSubsystemEOS_RoomSystem_MemoryLeakTest::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Rooms::Requests;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Core::Id;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create a shared pointer to hold the room ID.
    TSharedRef<FRoomIdPtr> RoomId = MakeShared<FRoomIdPtr>();
    FGuid SearchGuid = FGuid::NewGuid();

    for (size_t i = 0; i < 25; i++)
    {
        // Create a lobby with the room system.
        Executor->Then<FCreateRoomChainedTask>(
            0,
            SearchGuid,
            RoomId,
            TDelegate<void(FCreateRoomRequest &)>::CreateLambda([](FCreateRoomRequest &CreateRoomRequest) {
                CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
            }));

        // Update the lobby with a member and non-member attribute.
        Executor->Then<FUpdateRoomChainedTask>(
            0,
            TArray<int32>{0},
            RoomId,
            TDelegate<void(FUpdateRoomRequest &)>::CreateLambda([](FUpdateRoomRequest &UpdateRoomRequest) {
                UpdateRoomRequest.UpsertAttributes.Add(TEXT("Hello"), FString(TEXT("World")));
                UpdateRoomRequest.UpsertMemberAttributes.Add(TEXT("HelloMember"), FString(TEXT("WorldMember")));
            }),
            TDelegate<void(const FChainedTaskContextRef &Context, const FRoomRef &CurrentRoom)>());

        // Update the lobby with just a non-member attribute.
        Executor->Then<FUpdateRoomChainedTask>(
            0,
            TArray<int32>{0},
            RoomId,
            TDelegate<void(FUpdateRoomRequest &)>::CreateLambda([](FUpdateRoomRequest &UpdateRoomRequest) {
                UpdateRoomRequest.UpsertAttributes.Add(TEXT("Hello"), FString(TEXT("WorldUpdated")));
                UpdateRoomRequest.UpsertAttributes.Add(TEXT("HelloNew"), FString(TEXT("WorldNew")));
            }),
            TDelegate<void(const FChainedTaskContextRef &Context, const FRoomRef &CurrentRoom)>());

        // Search and join the lobby from user 1.
        Executor->Then<FJoinRoomViaSearchChainedTask>(1, TSet<int32>{0}, SearchGuid, RoomId, true);

        // Ensure the instances are in the expected state.
        Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0, 1}, TSet<int32>(), RoomId);

        // Leave the lobby.
        Executor->Then<FLeaveRoomChainedTask>(
            0,
            TMap<int32, ERoomLeaveReason>{{0, ERoomLeaveReason::LeftExplicitly}, {1, ERoomLeaveReason::RoomDestroyed}},
            TSet<int32>(),
            RoomId);
    }

    // Create a lobby with the room system.
    Executor->Then<FCreateRoomChainedTask>(
        0,
        SearchGuid,
        RoomId,
        TDelegate<void(FCreateRoomRequest &)>::CreateLambda([](FCreateRoomRequest &CreateRoomRequest) {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        }));

    // Update the lobby with a member and non-member attribute.
    Executor->Then<FUpdateRoomChainedTask>(
        0,
        TArray<int32>{0},
        RoomId,
        TDelegate<void(FUpdateRoomRequest &)>::CreateLambda([](FUpdateRoomRequest &UpdateRoomRequest) {
            UpdateRoomRequest.UpsertAttributes.Add(TEXT("Hello"), FString(TEXT("World")));
            UpdateRoomRequest.UpsertMemberAttributes.Add(TEXT("HelloMember"), FString(TEXT("WorldMember")));
        }),
        TDelegate<void(const FChainedTaskContextRef &Context, const FRoomRef &CurrentRoom)>());

    // Update the lobby with just a non-member attribute.
    Executor->Then<FUpdateRoomChainedTask>(
        0,
        TArray<int32>{0},
        RoomId,
        TDelegate<void(FUpdateRoomRequest &)>::CreateLambda([](FUpdateRoomRequest &UpdateRoomRequest) {
            UpdateRoomRequest.UpsertAttributes.Add(TEXT("Hello"), FString(TEXT("WorldUpdated")));
            UpdateRoomRequest.UpsertAttributes.Add(TEXT("HelloNew"), FString(TEXT("WorldNew")));
        }),
        TDelegate<void(const FChainedTaskContextRef &Context, const FRoomRef &CurrentRoom)>());

    for (size_t i = 0; i < 25; i++)
    {
        // Search and join the lobby from user 1.
        Executor->Then<FJoinRoomViaSearchChainedTask>(1, TSet<int32>{0}, SearchGuid, RoomId, true);

        // Ensure the instances are in the expected state.
        Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0, 1}, TSet<int32>(), RoomId);

        // Leave the lobby.
        Executor->Then<FLeaveRoomChainedTask>(
            1,
            TMap<int32, ERoomLeaveReason>{{1, ERoomLeaveReason::LeftExplicitly}},
            TSet<int32>(),
            RoomId);
    }

    // Run the test.
    Executor->Execute();
}

}