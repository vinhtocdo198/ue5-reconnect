// Copyright June Rhodes. All Rights Reserved.

// @note: This test is too flaky to enable, since it relies on the presence system querying lobbies after the EOS
// backend has replicated the new lobby state to search queries, and we don't know how long that will take. We could
// make this test retry periodically, but the refresh period for presence is around 10 seconds, so that will make this
// test take ages to complete if we have to wait for the backend to reconcile.
#if 0

#include "Internationalization/TextLocalizationManager.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSTests/Presence/PresenceTestLocalizedTextSource.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(3768096842, Redpoint::EOS::Tests::Presence)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Tests::Presence;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Presence_UpdatePresenceInterest,
    "Redpoint.EOS.Presence.UpdatePresenceInterest",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Presence_UpdatePresenceInterest::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Rooms;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Call SetPresence for the first user.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto PresenceSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

        // Register a text localization source so that presence can restore the StatusTextFormat from FText::FindText
        // (which usually only works if you prepare localization for the game).
        if (!FPresenceTestLocalizedTextSource::bRegistered)
        {
            FTextLocalizationManager::Get().RegisterTextSource(MakeShared<FPresenceTestLocalizedTextSource>());
            FPresenceTestLocalizedTextSource::bRegistered = true;
        }

        auto DesiredState = FPresenceUserState(GetAccountId(Context->User(0)));
        DesiredState.ActivityState = EPresenceUserActivityState::Online;
        DesiredState.StatusTextFormat = NSLOCTEXT("RedpointEOSTests", "StatusTextTest", "Status number is {Number}");
        DesiredState.StatusTextFormatArguments.Add(TEXT("Number"), FFormatArgumentValue(20));
        DesiredState.CustomAttributes.Add(TEXT("TestAttribute"), FRoomAttribute(TEXT("Hello World")));

        PresenceSystem->SetPresence(
            GetAccountId(Context->User(0)),
            DesiredState,
            IPresenceSystem::FOnSetPresenceComplete::CreateLambda(
                [PresenceSystem, Context, OnDone](const FError &ErrorCode) {
                    OnDone.ExecuteIfBound(Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected SetPresence operation to succeed."), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful()));
                }));
    });

    // Check that the second user can call UpdatePresenceInterest, see the presence data, and then call GetPresence to
    // also see it.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto PresenceSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(1)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

        PresenceSystem->UpdatePresenceInterest(
            FName(TEXT("AutomationTests")),
            GetAccountId(Context->User(1)),
            TArray<FPresenceInterestChange>{FPresenceInterestChange(GetAccountId(Context->User(0)), true)},
            true,
            IPresenceSystem::FOnUpdatePresenceInterestComplete::CreateLambda(
                [PresenceSystem, Context, OnDone](
                    const FError &ErrorCode,
                    const TMap<FIdentityUserId, FPresenceUserState> &RemoteUserPresenceState) {
                    bool bContinue = Context->Assert().TestTrue(
                        FString::Printf(
                            TEXT("Expected UpdatePresenceInterest operation to succeed."),
                            *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    if (!bContinue)
                    {
                        OnDone.ExecuteIfBound(bContinue);
                        return;
                    }

                    auto RunAssertions = [Context](const FPresenceUserState &State) {
                        Context->Assert().TestEqual(
                            TEXT("Expected ActivityState to be Online"),
                            State.ActivityState,
                            EPresenceUserActivityState::Online);
#if REDPOINT_EOS_UE_5_5_OR_LATER
                        Context->Assert().TestEqual(
                            TEXT("Expected StatusTextFormat to have text key StatusTextTest"),
                            FString(
                                FTextInspector::GetTextId(State.StatusTextFormat.GetSourceText()).GetKey().ToString()),
                            TEXT("StatusTextTest"));
#else
                        Context->Assert().TestEqual(
                            TEXT("Expected StatusTextFormat to have text key StatusTextTest"),
                            FString(
                                FTextInspector::GetTextId(State.StatusTextFormat.GetSourceText()).GetKey().GetChars()),
                            TEXT("StatusTextTest"));
#endif
                        Context->Assert().TestEqual(
                            TEXT("Expected StatusText to format correctly"),
                            State.GetStatusTextFormatted().ToString(),
                            TEXT("Status number is 20"));
                        Context->Assert().TestTrue(
                            TEXT("Expected to have TestAttribute custom attribute"),
                            State.CustomAttributes.Contains(TEXT("TestAttribute")));
                        // @todo: More state assertions.
                    };

                    auto *RemoteUser = RemoteUserPresenceState.Find(GetAccountId(Context->User(0)));
                    if (!Context->Assert().TestNotNull(
                            TEXT("Expected remote user presence state to have an entry for the desired user"),
                            RemoteUser))
                    {
                        OnDone.ExecuteIfBound(false);
                        return;
                    }
                    RunAssertions(*RemoteUser);

                    auto RetrievedState =
                        PresenceSystem->GetPresence(GetAccountId(Context->User(1)), GetAccountId(Context->User(0)));
                    if (!Context->Assert().TestTrue(
                            TEXT("Expected GetPresence to return a value for the remote user that we have just "
                                 "fetched presence for."),
                            RetrievedState.IsSet()))
                    {
                        OnDone.ExecuteIfBound(false);
                        return;
                    }
                    RunAssertions(RetrievedState.GetValue());

                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}

#endif