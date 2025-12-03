// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1236604457, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

static void ListenServer_RunAsyncTest(
    FAsyncHotReloadableAutomationTestBase *InTest,
    const FString &NetMode,
    const TFunction<void()> &OnDone)
{
    TSharedRef<FEditorChainedTaskExecutor> Executor = MakeShareable(new FEditorChainedTaskExecutor(
        InTest,
        OnDone,
        TArray<int32>{1, 1},
        EPlayNetMode::PIE_ListenServer,
        FString::Printf(TEXT("?%s"), *NetMode)));

    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FListenServer_OverIP,
    "Redpoint.EOS.Networking.OverIP.ListenServer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FListenServer_OverIP::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ListenServer_RunAsyncTest(this, TEXT("NetMode=ForceIP"), OnDone);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FListenServer_OverP2P,
    "Redpoint.EOS.Networking.OverP2P.ListenServer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FListenServer_OverP2P::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ListenServer_RunAsyncTest(this, TEXT("NetMode=ForceP2P"), OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()