// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"

#include "RedpointEOSTests/TestUtilities/MultiplayerScenarioInstance.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(436780220, Redpoint::EOS::Tests)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_TestHelpers_MultipleSubsystems,
    "Redpoint.EOS.TestHelpers.MultipleSubsystems",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_TestHelpers_MultipleSubsystems::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1, 1, 1, 1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](TArray<FMultiplayerScenarioInstance> Instances, const FOnDone &NestedOnDone) {
            TestEqual("Instance count is as expected", Instances.Num(), 4);
            for (int i = 0; i < Instances.Num(); i++)
            {
                TestTrue(FString::Printf(TEXT("Subsystem %d is valid"), i), Instances[i].Subsystem.IsValid());
                TestTrue(FString::Printf(TEXT("User %d is valid"), i), Instances[i].UserId.IsValid());
            }

            NestedOnDone();
        });
}

}

REDPOINT_EOS_CODE_GUARD_END()