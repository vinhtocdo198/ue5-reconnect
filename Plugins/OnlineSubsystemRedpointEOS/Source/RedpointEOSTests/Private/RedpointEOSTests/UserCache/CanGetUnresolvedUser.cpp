// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(3428306013, Redpoint::EOS::Tests::UserCache)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_UserCache_CanGetUnresolvedUser,
    "Redpoint.EOS.UserCache.CanGetUnresolvedUser",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_UserCache_CanGetUnresolvedUser::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::UserCache;
    using namespace ::Redpoint::EOS::Core::Id;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Get an unresolved user.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto UserCacheSystem =
                ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IUserCacheSystem>();

            auto TargetUser =
                UserCacheSystem->GetUnresolvedUser(GetAccountId(Context->User(0)), GetAccountId(Context->User(1)));
            Context->Assert().TestFalse(TEXT("Expected unresolved user to not be ready."), TargetUser->IsReady());
            Context->Assert().TestEqual(
                TEXT("Expected unresolved user ID to match target user ID."),
                TargetUser->GetUserId(),
                GetAccountId(Context->User(1)));
            OnDone.ExecuteIfBound(true);
        });

    // Run the test.
    Executor->Execute();
}

}