// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEChainedTask.h"
#include "RedpointEOSEditorTests/Networking/MakePIEStartListeningServerChainedTask.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(994650791, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Editor::Tests::Networking;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

static void ReconnectToSameServerImmediate_RunAsyncTest(
    FAsyncHotReloadableAutomationTestBase *InTest,
    const FString &NetMode,
    const TFunction<void()> &OnDone)
{
    TSharedRef<FEditorChainedTaskExecutor> Executor = MakeShareable(new FEditorChainedTaskExecutor(
        InTest,
        OnDone,
        TArray<int32>{1, 1},
        EPlayNetMode::PIE_Standalone,
        FString::Printf(TEXT("?%s"), *NetMode)));

    Executor->Then<FMakePIEStartListeningServerChainedTask>(0, TestHelpers::Port(9000), NetMode);

    Executor->Then<FConnectClientPIEToHostPIEChainedTask>(0, 1, TestHelpers::Port(9000));
    Executor->Then<FConnectClientPIEToHostPIEChainedTask>(0, 1, TestHelpers::Port(9000));

    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FReconnectToSameServerImmediate_OverIP,
    "Redpoint.EOS.Networking.OverIP.ReconnectToSameServerImmediate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FReconnectToSameServerImmediate_OverIP::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ReconnectToSameServerImmediate_RunAsyncTest(this, TEXT("NetMode=ForceIP"), OnDone);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FReconnectToSameServerImmediate_OverP2P,
    "Redpoint.EOS.Networking.OverP2P.ReconnectToSameServerImmediate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FReconnectToSameServerImmediate_OverP2P::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ReconnectToSameServerImmediate_RunAsyncTest(this, TEXT("NetMode=ForceP2P"), OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()