// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineIdentityInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSTests/VoiceChat/VoiceChatUserHolder.h"
#include "TestHelpers.h"
#include "VoiceChat.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2609470147, Redpoint::EOS::Tests::VoiceChat::Manual)
{
using namespace ::Redpoint::EOS::Tests::Party;
using namespace ::Redpoint::EOS::Tests::VoiceChat;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_VoiceChat_Manual_PartyVoice,
    "Redpoint.EOS.VoiceChat.Manual.PartyVoice",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter | EAutomationTestFlags::RequiresUser);

void FOnlineSubsystemEOS_VoiceChat_Manual_PartyVoice::RunAsyncTest(const TFunction<void()> &OnDone)
{
    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    auto VoiceChatUser = MakeShared<FVoiceChatUserHolder>();

    // Create a party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId, IOnlinePartySystem::GetPrimaryPartyTypeId(), true);

    // Login in the user to the voice chat system.
    Executor->Then<FLambdaChainedTask>([VoiceChatUser](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &TaskOnDone) {
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
                FOnVoiceChatLoginCompleteDelegate::CreateLambda([Context, TaskOnDone](
                                                                    const FString &PlayerName,
                                                                    const FVoiceChatResult &Result) {
                    UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Received callback from 'Login' on voice chat user."));
                    Context->Assert().TestTrue(TEXT("Expected voice chat to login successfully."), Result.IsSuccess());
                    TaskOnDone.ExecuteIfBound(true);
                }));
    });

    // Check that the voice chat system reports as having at least one channel.
    Executor->Then<FLambdaChainedTask>(
        [VoiceChatUser](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            Context->Assert().TestTrue(
                TEXT("Expected voice chat user to have at least one channel!"),
                (*VoiceChatUser)->GetChannels().Num() > 0);
            TaskOnDone.ExecuteIfBound(true);
        });

    // Log the user out of the voice chat system.
    Executor->Then<FLambdaChainedTask>([VoiceChatUser](
                                           const FChainedTaskContextRef &Context,
                                           const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Calling 'Logout' on voice chat user..."));
        (*VoiceChatUser)
            ->Logout(FOnVoiceChatLogoutCompleteDelegate::CreateLambda(
                [Context, TaskOnDone, VoiceChatUser /* unused but keeps shared ptr alive */](
                    const FString &PlayerName,
                    const FVoiceChatResult &Result) {
                    UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Received callback from 'Logout' on voice chat user."));
                    Context->Assert().TestTrue(TEXT("Expected voice chat to logout successfully."), Result.IsSuccess());
                    TaskOnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()