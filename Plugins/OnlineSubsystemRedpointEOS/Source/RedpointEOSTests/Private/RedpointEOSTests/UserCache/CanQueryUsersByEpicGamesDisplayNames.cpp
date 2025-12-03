// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(2945199250, Redpoint::EOS::Tests::UserCache)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_UserCache_CanQueryUsersByEpicGamesDisplayNames,
    "Redpoint.EOS.UserCache.CanQueryUsersByEpicGamesDisplayNames",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_UserCache_CanQueryUsersByEpicGamesDisplayNames::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::UserCache;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}, EChainedTaskExecutorFlag::EpicGames));

    // Resolve Epic Games display names.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto UserCacheSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IUserCacheSystem>();

        auto DisplayNames = TArray<FString>{
            TEXT("redpointqa0001"),
            TEXT("redpointqa0002"),
            TEXT("redpointqa0003"),
            TEXT("redpointqa0004")};
        UserCacheSystem->GetUsersByEpicGamesDisplayNames(
            GetAccountId(Context->User(0)),
            DisplayNames,
            IUserCacheSystem::FOnGetUsersByEpicGamesDisplayNamesComplete::CreateLambda(
                [Context, OnDone, DisplayNames](FError CallErrorCode, TMap<FString, FErrorableUserInfo> UserInfos) {
                    if (!Context->Assert().TestTrue(
                            FString::Printf(TEXT("Expected call to be successful: %s"), *CallErrorCode.ToLogString()),
                            CallErrorCode.WasSuccessful()))
                    {
                        OnDone.ExecuteIfBound(false);
                        return;
                    }

                    for (const auto &DisplayName : DisplayNames)
                    {
                        if (!Context->Assert().TestTrue(
                                FString::Printf(TEXT("Expected result map to contain key '%s'"), *DisplayName),
                                UserInfos.Contains(DisplayName)))
                        {
                            continue;
                        }
                        if (!Context->Assert().TestFalse(
                                FString::Printf(
                                    TEXT("Expected result map to contain key '%s': %s"),
                                    *DisplayName,
                                    *UserInfos[DisplayName].GetError()->ToLogString()),
                                UserInfos[DisplayName].IsErrored()))
                        {
                            continue;
                        }
                        Context->Assert().TestTrue(
                            FString::Printf(TEXT("Expected '%s' user to be ready"), *DisplayName),
                            UserInfos[DisplayName].GetUserInfo()->IsReady());
                        auto EpicGamesAccount = UserInfos[DisplayName].GetUserInfo()->GetLinkedEpicGamesAccount();
                        Context->Assert().TestTrue(
                            FString::Printf(
                                TEXT("Expected '%s' user to have Epic Games account information"),
                                *DisplayName),
                            EpicGamesAccount.IsSet());
                        Context->Assert().TestTrue(
                            FString::Printf(
                                TEXT("Expected '%s' user to have Epic Games account in linked external accounts"),
                                *DisplayName),
                            UserInfos[DisplayName].GetUserInfo()->GetLinkedExternalAccounts().Contains(
                                EOS_EExternalAccountType::EOS_EAT_EPIC));
                        Context->Assert().TestTrue(
                            FString::Printf(
                                TEXT("Expected '%s' user to have recently signed in information"),
                                *DisplayName),
                            UserInfos[DisplayName].GetUserInfo()->GetMostRecentlySignedInPlatform().IsSet());
                        Context->Assert().TestEqual(
                            FString::Printf(
                                TEXT("Expected '%s' user to have recently signed in with Epic Games"),
                                *DisplayName),
                            UserInfos[DisplayName].GetUserInfo()->GetMostRecentlySignedInPlatform().GetValue(),
                            EOS_EExternalAccountType::EOS_EAT_EPIC);
                    }

                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}