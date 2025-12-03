// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(691144252, Redpoint::EOS::Tests::UserCache)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_UserCache_CanQueryUsersByExternalMappings,
    "Redpoint.EOS.UserCache.CanQueryUsersByExternalMappings",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_UserCache_CanQueryUsersByExternalMappings::RunAsyncTest(const TFunction<void()> &OnDone)
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
        TargetUserIds.Add(GetAccountId(Context->User(0)));
        TargetUserIds.Add(GetAccountId(Context->User(1)));
        TargetUserIds.Add(GetAccountId(Context->User(2)));
        TargetUserIds.Add(GetAccountId(Context->User(3)));
        UserCacheSystem->GetUsers(
            GetAccountId(Context->User(0)),
            TargetUserIds,
            IUserCacheSystem::FOnGetUsersComplete::CreateLambda([Context, OnDone, TargetUserIds, UserCacheSystem](
                                                                    FError LookupCallErrorCode,
                                                                    TMap<UE::Online::FAccountId, FErrorableUserInfo>
                                                                        LookupUserInfos) {
                if (!Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected call to be successful: %s"), *LookupCallErrorCode.ToLogString()),
                        LookupCallErrorCode.WasSuccessful()) ||
                    !Context->Assert()
                         .TestEqual(TEXT("Expected to have 4 user infos returned"), LookupUserInfos.Num(), 4))
                {
                    OnDone.ExecuteIfBound(false);
                    return;
                }

                // Now do our actual query to find by external mappings.
                TArray<FString> OpenIdExternalIds;
                for (int i = 0; i < 4; i++)
                {
                    OpenIdExternalIds.Add(LookupUserInfos[GetAccountId(Context->User(i))]
                                              .GetUserInfo()
                                              ->GetLinkedExternalAccounts()[EOS_EExternalAccountType::EOS_EAT_OPENID]
                                              .AccountId);
                }

                UserCacheSystem->GetUsersByExternalAccountIds(
                    GetAccountId(Context->User(0)),
                    EOS_EExternalAccountType::EOS_EAT_OPENID,
                    OpenIdExternalIds,
                    IUserCacheSystem::FOnGetUsersByExternalAccountIdsComplete::CreateLambda(
                        [Context,
                         OnDone,
                         OpenIdExternalIds](FError CallErrorCode, TMap<FString, FErrorableUserInfo> UserInfos) {
                            if (!Context->Assert().TestTrue(
                                    FString::Printf(
                                        TEXT("Expected call to be successful: %s"),
                                        *CallErrorCode.ToLogString()),
                                    CallErrorCode.WasSuccessful()))
                            {
                                OnDone.ExecuteIfBound(false);
                                return;
                            }

                            for (const auto &ExternalId : OpenIdExternalIds)
                            {
                                if (!Context->Assert().TestTrue(
                                        FString::Printf(TEXT("Expected result map to contain key '%s'"), *ExternalId),
                                        UserInfos.Contains(ExternalId)))
                                {
                                    continue;
                                }
                                if (!Context->Assert().TestFalse(
                                        FString::Printf(
                                            TEXT("Expected result map to contain key '%s': %s"),
                                            *ExternalId,
                                            *UserInfos[ExternalId].GetError()->ToLogString()),
                                        UserInfos[ExternalId].IsErrored()))
                                {
                                    continue;
                                }
                                Context->Assert().TestTrue(
                                    FString::Printf(TEXT("Expected '%s' user to be ready"), *ExternalId),
                                    UserInfos[ExternalId].GetUserInfo()->IsReady());
                                auto EpicGamesAccount =
                                    UserInfos[ExternalId].GetUserInfo()->GetLinkedEpicGamesAccount();
                                Context->Assert().TestFalse(
                                    FString::Printf(
                                        TEXT("Expected '%s' user to not have Epic Games account information"),
                                        *ExternalId),
                                    EpicGamesAccount.IsSet());
                                Context->Assert().TestTrue(
                                    FString::Printf(
                                        TEXT("Expected '%s' user to have OpenID in linked external accounts"),
                                        *ExternalId),
                                    UserInfos[ExternalId].GetUserInfo()->GetLinkedExternalAccounts().Contains(
                                        EOS_EExternalAccountType::EOS_EAT_OPENID));
                                Context->Assert().TestTrue(
                                    FString::Printf(
                                        TEXT("Expected '%s' user to have recently signed in information"),
                                        *ExternalId),
                                    UserInfos[ExternalId].GetUserInfo()->GetMostRecentlySignedInPlatform().IsSet());
                                Context->Assert().TestEqual(
                                    FString::Printf(
                                        TEXT("Expected '%s' user to have recently signed in with OpenID"),
                                        *ExternalId),
                                    UserInfos[ExternalId].GetUserInfo()->GetMostRecentlySignedInPlatform().GetValue(),
                                    EOS_EExternalAccountType::EOS_EAT_OPENID);
                            }

                            OnDone.ExecuteIfBound(true);
                        }));
            }));
    });

    // Run the test.
    Executor->Execute();
}

}