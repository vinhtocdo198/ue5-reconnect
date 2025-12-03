// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(1092331693, Redpoint::EOS::Tests::UserCache)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_UserCache_CanQueryMultipleUsers,
    "Redpoint.EOS.UserCache.CanQueryMultipleUsers",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_UserCache_CanQueryMultipleUsers::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::UserCache;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1, 1, 1}));

    // Resolve multiple users.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto UserCacheSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IUserCacheSystem>();

        TArray<UE::Online::FAccountId> TargetUserIds;
        TargetUserIds.Add(GetAccountId(Context->User(1)));
        TargetUserIds.Add(GetAccountId(Context->User(2)));
        TargetUserIds.Add(GetAccountId(Context->User(3)));
        UserCacheSystem->GetUsers(
            GetAccountId(Context->User(0)),
            TargetUserIds,
            IUserCacheSystem::FOnGetUsersComplete::CreateLambda(
                [Context,
                 OnDone,
                 TargetUserIds](FError CallErrorCode, TMap<UE::Online::FAccountId, FErrorableUserInfo> UserInfos) {
                    if (!Context->Assert().TestTrue(
                            FString::Printf(TEXT("Expected call to be successful: %s"), *CallErrorCode.ToLogString()),
                            CallErrorCode.WasSuccessful()))
                    {
                        OnDone.ExecuteIfBound(false);
                        return;
                    }

                    for (const auto &TargetUserId : TargetUserIds)
                    {
                        if (!Context->Assert().TestTrue(
                                FString::Printf(
                                    TEXT("Expected result map to contain key '%s'"),
                                    *GetUserIdString(TargetUserId)),
                                UserInfos.Contains(TargetUserId)))
                        {
                            continue;
                        }
                        if (!Context->Assert().TestFalse(
                                FString::Printf(
                                    TEXT("Expected result map to contain key '%s': %s"),
                                    *GetUserIdString(TargetUserId),
                                    *UserInfos[TargetUserId].GetError()->ToLogString()),
                                UserInfos[TargetUserId].IsErrored()))
                        {
                            continue;
                        }
                        Context->Assert().TestTrue(
                            FString::Printf(TEXT("Expected '%s' user to be ready"), *GetUserIdString(TargetUserId)),
                            UserInfos[TargetUserId].GetUserInfo()->IsReady());
                        auto EpicGamesAccount = UserInfos[TargetUserId].GetUserInfo()->GetLinkedEpicGamesAccount();
                        Context->Assert().TestFalse(
                            FString::Printf(
                                TEXT("Expected '%s' user to not have Epic Games account information"),
                                *GetUserIdString(TargetUserId)),
                            EpicGamesAccount.IsSet());
                        Context->Assert().TestTrue(
                            FString::Printf(
                                TEXT("Expected '%s' user to have OpenID in linked external accounts"),
                                *GetUserIdString(TargetUserId)),
                            UserInfos[TargetUserId].GetUserInfo()->GetLinkedExternalAccounts().Contains(
                                EOS_EExternalAccountType::EOS_EAT_OPENID));
                        Context->Assert().TestTrue(
                            FString::Printf(
                                TEXT("Expected '%s' user to have recently signed in information"),
                                *GetUserIdString(TargetUserId)),
                            UserInfos[TargetUserId].GetUserInfo()->GetMostRecentlySignedInPlatform().IsSet());
                        Context->Assert().TestEqual(
                            FString::Printf(
                                TEXT("Expected '%s' user to have recently signed in with OpenID"),
                                *GetUserIdString(TargetUserId)),
                            UserInfos[TargetUserId].GetUserInfo()->GetMostRecentlySignedInPlatform().GetValue(),
                            EOS_EExternalAccountType::EOS_EAT_OPENID);
                    }

                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}