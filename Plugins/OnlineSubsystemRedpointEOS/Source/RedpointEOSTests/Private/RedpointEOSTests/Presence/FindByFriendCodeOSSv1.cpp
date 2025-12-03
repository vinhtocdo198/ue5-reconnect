// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(3915948696, Redpoint::EOS::Tests::Presence)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Presence_FindByFriendCodeOSSv1,
    "Redpoint.EOS.Presence.FindByFriendCodeOSSv1",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Presence_FindByFriendCodeOSSv1::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Attempt to find our second user by friend code.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            FString FriendCode;
            if (!Context->Assert().TestTrue(
                    TEXT("Expected to be able to get 'friendCode' attribute via OSSv1"),
                    Context->Instance(1)
                        .GetIdentityInterface()
                        ->GetUserAccount(Context->User(1))
                        ->GetAuthAttribute(TEXT("friendCode"), FriendCode)))
            {
                OnDone.ExecuteIfBound(false);
                return;
            }
            if (!Context->Assert().TestTrue(
                    TEXT("Expected friend code value to not be an empty string"),
                    !FriendCode.IsEmpty()))
            {
                OnDone.ExecuteIfBound(false);
                return;
            }

            if (!Context->Assert().TestTrue(
                    TEXT("Expected QueryUserIdMapping operation to start"),
                    Context->Instance(0).GetUserInterface()->QueryUserIdMapping(
                        Context->User(0),
                        FString::Printf(TEXT("FriendCode:%s"), *FriendCode),
                        IOnlineUser::FOnQueryUserMappingComplete::CreateLambda([FriendCode, Context, OnDone](
                                                                                   bool bWasSuccessful,
                                                                                   const FUniqueNetId &UserId,
                                                                                   const FString &DisplayNameOrEmail,
                                                                                   const FUniqueNetId &FoundUserId,
                                                                                   const FString &Error) {
                            Context->Assert().TestTrue(TEXT("Expected operation to be successful"), bWasSuccessful);
                            Context->Assert().TestEqual(
                                TEXT("Expected display name or email to match input"),
                                DisplayNameOrEmail,
                                FString::Printf(TEXT("FriendCode:%s"), *FriendCode));
                            Context->Assert().TestTrue(
                                TEXT("Expected found user ID to match user 1"),
                                FoundUserId == Context->User(1));
                            Context->Assert().TestTrue(TEXT("Expected error text to be empty"), Error.IsEmpty());
                            OnDone.ExecuteIfBound(true);
                        }))))
            {
                OnDone.ExecuteIfBound(false);
                return;
            }
        });

    // Run the test.
    Executor->Execute();
}

}