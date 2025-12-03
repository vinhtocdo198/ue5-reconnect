// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(2210323961, Redpoint::EOS::Tests::RoomSystem)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbyAlias,
    "Redpoint.EOS.RoomSystem.LobbyAlias",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbyAlias::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Rooms::Requests;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Tests::RoomSystem;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create a shared pointer to hold the room ID.
    TSharedRef<FRoomIdPtr> RoomId = MakeShared<FRoomIdPtr>();
    FGuid SearchGuid = FGuid::NewGuid();

    // Make sure creating a lobby with an alias works.
    Executor->Then<FLambdaChainedTask>([RoomId](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &OnDone) {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(0)).GetRoomSystem();

        auto CreateRoomRequest =
            FCreateRoomRequest(ERoomScope::Global, GetAccountId(Context->User(0)), NAME_Lobby, NAME_AutomationTests);
        CreateRoomRequest.AddFeatureRequest(MakeShared<FAliasRoomFeatureRequest>(TEXT("GameSession")));

        RoomSystem->CreateRoom(
            CreateRoomRequest,
            IRoomSystem::FOnCreateRoomComplete::CreateLambda(
                [RoomId, Context, OnDone](FError ErrorCode, FRoomPtr Room) {
                    bool bContinue = Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected CreateRoom with alias to succeed."), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    OnDone.ExecuteIfBound(bContinue);
                }));
    });

    // Make sure leaving the lobby works.
    Executor->Then<FLambdaChainedTask>(
        [RoomId](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(0)).GetRoomSystem();

            auto CurrentRooms =
                RoomSystem->GetCurrentRooms(FGetCurrentRoomsRequest(ERoomScope::Global, NAME_AutomationTests));
            if (!Context->Assert().TestTrue(TEXT("Expected one room"), CurrentRooms.Num() == 1))
            {
                OnDone.ExecuteIfBound(false);
                return;
            }

            auto LeaveRoomRequest = FLeaveRoomRequest();
            RoomSystem->LeaveRoom(
                CurrentRooms[0],
                LeaveRoomRequest,
                IRoomSystem::FOnLeaveRoomComplete::CreateLambda([RoomId, Context, OnDone](FError ErrorCode) {
                    bool bContinue = Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected LeaveRoom to succeed."), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    OnDone.ExecuteIfBound(bContinue);
                }));
        });

    // Run the test.
    Executor->Execute();
}

}