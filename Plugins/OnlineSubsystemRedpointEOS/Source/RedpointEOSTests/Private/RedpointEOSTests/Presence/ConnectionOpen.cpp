// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSTests/TestUtilities/LambdaCommand.h"

namespace REDPOINT_EOS_FILE_NS_ID(419385808, Redpoint::EOS::Tests::Presence)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Presence_ConnectionOpen,
    "Redpoint.EOS.Presence.ConnectionOpen",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Presence_ConnectionOpen::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Rooms;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Establish a connection and close it.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto PresenceSystem =
                ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

            // Open a connection to the second user, then close it.
            PresenceSystem->TryOpenMessageConnection(
                GetAccountId(Context->User(0)),
                GetAccountId(Context->User(1)),
                FName(TEXT("AutomatedTests")),
                IPresenceSystem::FOnTryOpenMessageConnection::CreateLambda(
                    [Context,
                     OnDone](const FError &ErrorCode, const TSharedPtr<IPresenceMessageConnection> &MessageConnection) {
                        Context->Assert().TestTrue(
                            FString::Printf(
                                TEXT("Expected TryOpenMessageConnection operation to succeed."),
                                *ErrorCode.ToLogString()),
                            ErrorCode.WasSuccessful());
                        if (Context->Assert().TestTrue(
                                TEXT("Expected MessageConnection to not be null."),
                                MessageConnection.IsValid()))
                        {
                            MessageConnection->Close(IPresenceMessageConnection::FOnCloseComplete::CreateLambda(
                                [Context, OnDone](const FError &CloseErrorCode) {
                                    Context->Assert().TestTrue(
                                        FString::Printf(
                                            TEXT("Expected Close operation to succeed."),
                                            *CloseErrorCode.ToLogString()),
                                        CloseErrorCode.WasSuccessful());
                                    OnDone.ExecuteIfBound(true);
                                }));
                        }
                    }));
        });

    // Run the test.
    Executor->Execute();
}

}