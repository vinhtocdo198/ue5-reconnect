// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(1397177333, Redpoint::EOS::Tests::Presence)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Presence_FindByFriendCode,
    "Redpoint.EOS.Presence.FindByFriendCode",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Presence_FindByFriendCode::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::UserCache;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Attempt to find our second user by friend code.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto PresenceSystem0 =
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IPresenceSystem>();
        auto PresenceSystem1 =
            ((FOnlineSubsystemEOS &)Context->Instance(1)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

        auto FriendCode = PresenceSystem1->GetFriendCode(GetAccountId(Context->User(1)));
        if (!Context->Assert().TestTrue(TEXT("Friend code is set"), FriendCode.IsSet()))
        {
            OnDone.ExecuteIfBound(false);
            return;
        }

        PresenceSystem0->FindByFriendCode(
            GetAccountId(Context->User(0)),
            FriendCode.GetValue(),
            IPresenceSystem::FOnFindByFriendCodeComplete::CreateLambda(
                [Context, OnDone](const FError &ErrorCode, const TArray<FUserInfoRef> &DiscoveredUsers) {
                    Context->Assert().TestTrue(
                        FString::Printf(TEXT("Find by friend code search successful: %s"), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    if (Context->Assert()
                            .TestEqual(TEXT("Expected to find exactly one user"), 1, DiscoveredUsers.Num()))
                    {
                        Context->Assert().TestTrue(
                            TEXT("Expected found user ID to match"),
                            DiscoveredUsers[0]->GetUserId() == GetAccountId(Context->User(1)));
                    }
                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}