// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(1067740816, Redpoint::EOS::Tests::UserCache)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_UserCache_CanQuerySingleUser,
    "Redpoint.EOS.UserCache.CanQuerySingleUser",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_UserCache_CanQuerySingleUser::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::UserCache;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Resolve a user.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto UserCacheSystem =
                ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IUserCacheSystem>();

            UserCacheSystem->GetUser(
                GetAccountId(Context->User(0)),
                GetAccountId(Context->User(1)),
                IUserCacheSystem::FOnGetUserComplete::CreateLambda([Context,
                                                                    OnDone](FError ErrorCode, FUserInfoPtr UserInfo) {
                    bool bWasSuccessful = Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected user to resolve: %s"), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    bool bWasNotNull =
                        Context->Assert().TestNotNull(TEXT("Expected user info to not be nullptr."), UserInfo.Get());
                    if (!bWasSuccessful || !bWasNotNull)
                    {
                        OnDone.ExecuteIfBound(false);
                        return;
                    }

                    Context->Assert().TestEqual(
                        TEXT("Expected target user ID to match requested user ID"),
                        UserInfo->GetUserId(),
                        GetAccountId(Context->User(1)));
                    Context->Assert().TestTrue(TEXT("Expected target user ID to be ready"), UserInfo->IsReady());
                    Context->Assert().TestTrue(
                        TEXT("Expected target user ID to have attributes"),
                        UserInfo->GetAttributes().Num() > 0);
                    OnDone.ExecuteIfBound(true);
                }));
        });

    // Run the test.
    Executor->Execute();
}

}