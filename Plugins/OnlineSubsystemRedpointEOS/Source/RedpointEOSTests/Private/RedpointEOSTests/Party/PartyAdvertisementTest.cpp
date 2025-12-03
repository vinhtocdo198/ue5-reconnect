// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(566229226, Redpoint::EOS::Tests::Party)
{

using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_Advertisement,
    "Redpoint.EOS.Party.Advertisement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_Advertisement::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Identity;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId1 = MakeShared<TSharedPtr<const FOnlinePartyId>>();

    // Create the party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId1);

    // From the second user, get the presence system, register interest and wait for that to be received. We have to do
    // this because GetAdvertisedParty is synchronous and getting the initial presence takes time.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            auto PresenceSystem =
                ((FOnlineSubsystemEOS &)Context->Instance(1)).GetPlatformHandle()->GetSystem<IPresenceSystem>();
            PresenceSystem->UpdatePresenceInterest(
                FName(TEXT("AutomationTests")),
                GetAccountId(Context->User(1)),
                TArray<FPresenceInterestChange>{FPresenceInterestChange(GetAccountId(Context->User(0)), true)},
                true,
                IPresenceSystem::FOnUpdatePresenceInterestComplete::CreateLambda(
                    [PresenceSystem, Context, TaskOnDone](
                        const FError &ErrorCode,
                        const TMap<FIdentityUserId, FPresenceUserState> &RemoteUserPresenceState) {
                        bool bContinue = Context->Assert().TestTrue(
                            FString::Printf(
                                TEXT("Expected UpdatePresenceInterest operation to succeed."),
                                *ErrorCode.ToLogString()),
                            ErrorCode.WasSuccessful());
                        if (!bContinue)
                        {
                            TaskOnDone.ExecuteIfBound(bContinue);
                            return;
                        }

                        TaskOnDone.ExecuteIfBound(true);
                    }));
        });

    // Verify that the second user can see the advertised party of the first party.
    Executor->Then<FLambdaChainedTask>(
        [StoredPartyId1](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            auto JoinInfo = Context->Instance(1).GetPartyInterface()->GetAdvertisedParty(
                Context->User(1),
                Context->User(0),
                Context->Instance(1).GetPartyInterface()->GetPrimaryPartyTypeId());
            if (Context->Assert().TestTrue(TEXT("Expected JoinInfo to be valid."), JoinInfo.IsValid()))
            {
                Context->Assert().TestEqual(
                    TEXT("Expected party ID of join info be correct."),
                    *JoinInfo->GetPartyId(),
                    **StoredPartyId1);
            }
            TaskOnDone.ExecuteIfBound(true);
        });

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()