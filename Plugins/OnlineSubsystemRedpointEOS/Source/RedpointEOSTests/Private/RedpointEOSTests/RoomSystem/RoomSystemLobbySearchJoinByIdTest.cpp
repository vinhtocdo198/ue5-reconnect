// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/CreateRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsurePublicMemberListChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsureRoomMembersChainedTask.h"
#include "RedpointEOSTests/RoomSystem/JoinRoomViaSearchChainedTask.h"
#include "RedpointEOSTests/RoomSystem/LeaveRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/PromoteRoomMemberChainedTask.h"
#include "RedpointEOSTests/RoomSystem/SearchRoomsChainedTask.h"
#include "RedpointEOSTests/RoomSystem/UpdateRoomChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(2964516581, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbySearchJoinById,
    "Redpoint.EOS.RoomSystem.LobbySearchJoinById",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbySearchJoinById::RunAsyncTest(const TFunction<void()> &OnDone)
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

    // Create a lobby with the room system.
    Executor->Then<FCreateRoomChainedTask>(
        0,
        SearchGuid,
        RoomId,
        TDelegate<void(FCreateRoomRequest &)>::CreateLambda([SearchGuid](FCreateRoomRequest &CreateRoomRequest) {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
            CreateRoomRequest.Attributes.Add(TEXT("Guid"), SearchGuid.ToString());
        }));

    // Make sure we can find the lobby by the room ID.
    Executor->Then<FSearchRoomsChainedTask>(
        1,
        RoomId,
        TDelegate<void(const FChainedTaskContextRef &, const TArray<FRoomSearchResultRef> &)>::CreateLambda(
            [](const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &Results) {
                Context->Assert().TestTrue(TEXT("Expected to find search result by GUID only!"), Results.Num() > 0);
            }));

    // Leave the lobby.
    Executor->Then<FLeaveRoomChainedTask>(
        0,
        TMap<int32, ERoomLeaveReason>{{0, ERoomLeaveReason::LeftExplicitly}},
        TSet<int32>(),
        RoomId);

    // Run the test.
    Executor->Execute();
}

}