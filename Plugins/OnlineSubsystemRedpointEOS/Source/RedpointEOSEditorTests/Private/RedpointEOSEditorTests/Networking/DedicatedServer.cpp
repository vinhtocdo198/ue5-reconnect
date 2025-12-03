// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1763466835, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

static void DedicatedServer_RunAsyncTest(
    FAsyncHotReloadableAutomationTestBase *InTest,
    const FString &NetMode,
    const TFunction<void()> &OnDone)
{
    TSharedRef<FEditorChainedTaskExecutor> Executor = MakeShareable(new FEditorChainedTaskExecutor(
        InTest,
        OnDone,
        TArray<int32>{1},
        EPlayNetMode::PIE_Client,
        FString::Printf(TEXT("?%s"), *NetMode)));

    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FDedicatedServer_OverIP,
    "Redpoint.EOS.Networking.OverIP.DedicatedServer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FDedicatedServer_OverIP::RunAsyncTest(const TFunction<void()> &OnDone)
{
    DedicatedServer_RunAsyncTest(this, TEXT("NetMode=ForceIP"), OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()