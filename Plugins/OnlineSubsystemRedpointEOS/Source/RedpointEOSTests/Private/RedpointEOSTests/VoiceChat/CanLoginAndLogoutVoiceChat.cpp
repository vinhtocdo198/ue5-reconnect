// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineIdentityInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSTests/VoiceChat/VoiceChatUserHolder.h"
#include "TestHelpers.h"
#include "VoiceChat.h"

namespace REDPOINT_EOS_FILE_NS_ID(3723700538, Redpoint::EOS::Tests::VoiceChat)
{
using namespace ::Redpoint::EOS::Tests::VoiceChat;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_VoiceChat_CanLoginAndLogoutVoiceChat,
    "Redpoint.EOS.VoiceChat.CanLoginAndLogoutVoiceChat",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_VoiceChat_CanLoginAndLogoutVoiceChat::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    // Create a shared pointer that we can use to store the IVoiceChatUser*.
    auto VoiceChatUser = MakeShared<FVoiceChatUserHolder>();

    // Login in the user to the voice chat system.
    Executor->Then<FLambdaChainedTask>([VoiceChatUser](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &OnDone) {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Getting voice chat interface and creating voice chat user..."));
        IVoiceChat *VoiceChat = IVoiceChat::Get();
        *VoiceChatUser = VoiceChat->CreateUser();

        FPlatformUserId PlatformUserId =
            Context->Instance(0).GetIdentityInterface()->GetPlatformUserIdFromUniqueNetId(Context->User(0));

        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Calling 'Login' on voice chat user..."));
        (*VoiceChatUser)
            ->Login(
                PlatformUserId,
                Context->Instance(0).GetIdentityInterface()->GetUniquePlayerId(0)->ToString(),
                TEXT(""),
                FOnVoiceChatLoginCompleteDelegate::CreateLambda([Context, OnDone](
                                                                    const FString &PlayerName,
                                                                    const FVoiceChatResult &Result) {
                    UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Received callback from 'Login' on voice chat user."));
                    Context->Assert().TestTrue(TEXT("Expected voice chat to login successfully."), Result.IsSuccess());
                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Log the user out of the voice chat system.
    Executor->Then<FLambdaChainedTask>([VoiceChatUser](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &OnDone) {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Calling 'Logout' on voice chat user..."));
        (*VoiceChatUser)
            ->Logout(FOnVoiceChatLogoutCompleteDelegate::CreateLambda(
                [Context, OnDone, VoiceChatUser /* unused but keeps shared ptr alive */](
                    const FString &PlayerName,
                    const FVoiceChatResult &Result) {
                    UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Received callback from 'Logout' on voice chat user."));
                    Context->Assert().TestTrue(TEXT("Expected voice chat to logout successfully."), Result.IsSuccess());
                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}