// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineIdentityInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(3204709027, Redpoint::EOS::Tests::Identity)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Identity_PlatformUserIdRoundtrips,
    "Redpoint.EOS.Identity.PlatformUserIdRoundtrips",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Identity_PlatformUserIdRoundtrips::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    // Login in the user to the voice chat system.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto IdentityInterface = Context->Instance(0).GetIdentityInterface();

            auto UserId = IdentityInterface->GetUniquePlayerId(0);

            auto PlatformUserId = IdentityInterface->GetPlatformUserIdFromUniqueNetId(*UserId);
            auto LoadedLocalUserNum = IdentityInterface->GetLocalUserNumFromPlatformUserId(PlatformUserId);
            auto LoadedPlatformUserId = IdentityInterface->GetPlatformUserIdFromLocalUserNum(LoadedLocalUserNum);

            Context->Assert().TestEqual(
                TEXT("Expected GetLocalUserNumFromPlatformUserId to return 0"),
                LoadedLocalUserNum,
                0);
            Context->Assert().TestEqual(
                TEXT("Expected GetPlatformUserIdFromLocalUserNum to return the same value as "
                     "GetPlatformUserIdFromUniqueNetId"),
                LoadedPlatformUserId,
                PlatformUserId);

            OnDone.ExecuteIfBound(true);
        });

    // Run the test.
    Executor->Execute();
}

}