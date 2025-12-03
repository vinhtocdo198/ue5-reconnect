// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(1233406731, Redpoint::EOS::Tests::RoomSystem)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbyRestoreInvites,
    "Redpoint.EOS.RoomSystem.LobbyRestoreInvites",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbyRestoreInvites::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Rooms::Requests;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    // Restore invites and make sure we were successful.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(0)).GetRoomSystem();
            RoomSystem->RestoreRoomInvites(
                FRestoreRoomInvitesRequest{GetAccountId(Context->User(0))},
                IRoomSystem::FOnRestoreRoomInvitesComplete::CreateLambda([Context, OnDone](FError ErrorCode) {
                    Context->Assert().TestTrue(
                        FString::Printf(
                            TEXT("Expected RestoreRoomInvites operation to succeed: %s"),
                            *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    OnDone.ExecuteIfBound(ErrorCode.WasSuccessful());
                }));
        });

    // Run the test.
    Executor->Execute();
}

}