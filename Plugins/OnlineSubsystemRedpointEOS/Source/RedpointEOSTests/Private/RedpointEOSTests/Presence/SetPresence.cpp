// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(2561371979, Redpoint::EOS::Tests::Presence)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Presence_SetPresence,
    "Redpoint.EOS.Presence.SetPresence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Presence_SetPresence::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Rooms;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    // Check that we can call SetPresence and then read our own presence with GetPresence.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto PresenceSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

        auto DesiredState = FPresenceUserState(GetAccountId(Context->User(0)));
        DesiredState.CustomAttributes.Add(TEXT("TestAttribute"), FRoomAttribute(TEXT("Hello World")));

        PresenceSystem->SetPresence(
            GetAccountId(Context->User(0)),
            DesiredState,
            IPresenceSystem::FOnSetPresenceComplete::CreateLambda(
                [PresenceSystem, Context, OnDone](const FError &ErrorCode) {
                    bool bContinue = Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected SetPresence operation to succeed."), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    if (!bContinue)
                    {
                        OnDone.ExecuteIfBound(bContinue);
                        return;
                    }

                    auto RetrievedState =
                        PresenceSystem->GetPresence(GetAccountId(Context->User(0)), GetAccountId(Context->User(0)));
                    if (Context->Assert().TestTrue(
                            TEXT("Expected GetPresence to return a value for the local user."),
                            RetrievedState.IsSet()))
                    {
                        if (Context->Assert().TestTrue(
                                TEXT("Expected returned presence value to contain custom TestAttribute attribute."),
                                RetrievedState.GetValue().CustomAttributes.Contains(TEXT("TestAttribute"))))
                        {
                            Context->Assert().TestEqual(
                                TEXT("Expected returned presence value to contain custom TestAttribute attribute."),
                                RetrievedState.GetValue().CustomAttributes[TEXT("TestAttribute")].GetString(),
                                TEXT("Hello World"));
                        }
                    }

                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}