// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/CreateRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsureRoomMembersChainedTask.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"
#include "RedpointEOSTests/RoomSystem/JoinRoomViaSearchChainedTask.h"
#include "RedpointEOSTests/RoomSystem/SendRoomInviteChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(2319870958, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::API;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbyInvite,
    "Redpoint.EOS.RoomSystem.LobbyInvite",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbyInvite::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Rooms::Requests;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Core::Id;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create a shared pointer to hold data.
    TSharedRef<FRoomIdPtr> RoomId = MakeShared<FRoomIdPtr>();
    FGuid SearchGuid = FGuid::NewGuid();
    TSharedRef<FRoomInvitePtr> RoomInvite = MakeShared<FRoomInvitePtr>();

    // Create a lobby with the room system.
    Executor->Then<FCreateRoomChainedTask>(
        0,
        SearchGuid,
        RoomId,
        TDelegate<void(FCreateRoomRequest &)>::CreateLambda([](FCreateRoomRequest &CreateRoomRequest) {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(false));
        }));

    // Send the room invite.
    Executor->Then<FSendRoomInviteChainedTask>(0, 1, RoomId, RoomInvite);

    // Attempt to join the room as user 1, consuming the invite.
    Executor->Then<FLambdaChainedTask>(
        [RoomInvite](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(1)).GetRoomSystem();
            RoomSystem->JoinRoom(
                FJoinRoomRequest(ERoomScope::User, RoomInvite->ToSharedRef()),
                IRoomSystem::FOnJoinRoomComplete::CreateLambda([Context, OnDone](FError ErrorCode, FRoomPtr Room) {
                    Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected JoinRoom operation to succeed: %s"), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    OnDone.ExecuteIfBound(ErrorCode.WasSuccessful());
                }));
        });

    // Ensure the instances are in the expected state.
    // Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0, 1}, TSet<int32>(), RoomId);

    // Run the test.
    Executor->Execute();
}

}