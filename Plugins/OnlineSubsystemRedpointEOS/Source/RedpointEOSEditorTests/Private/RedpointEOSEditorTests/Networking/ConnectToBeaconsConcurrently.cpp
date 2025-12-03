// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSEditorTests/Networking/CloseClientPIEBeaconChainedTask.h"
#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEBeaconChainedTask.h"
#include "RedpointEOSEditorTests/Networking/MakePIEStartListeningServerChainedTask.h"
#include "RedpointEOSEditorTests/Networking/SpawnBeaconOnPIEChainedTask.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4250119503, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Editor::Tests::Networking;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

static void ConnectToBeaconsConcurrently_RunAsyncTest(
    FAsyncHotReloadableAutomationTestBase *InTest,
    const FString &NetMode,
    const TFunction<void()> &OnDone)
{
    TSharedRef<FEditorChainedTaskExecutor> Executor = MakeShareable(new FEditorChainedTaskExecutor(
        InTest,
        OnDone,
        TArray<int32>{1, 1, 1},
        EPlayNetMode::PIE_Standalone,
        FString::Printf(TEXT("?%s"), *NetMode)));

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> BeaconHost1 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> BeaconHostObject1 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>>();

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> BeaconHost2 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> BeaconHostObject2 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>>();

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClientTo1 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClientTo2 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>>();

    Executor->Then<FMakePIEStartListeningServerChainedTask>(0, TestHelpers::Port(9000), NetMode);
    Executor->Then<FMakePIEStartListeningServerChainedTask>(1, TestHelpers::Port(9001), NetMode);

    Executor->Then<FSpawnBeaconOnPIEChainedTask>(BeaconHost1, BeaconHostObject1, 0, TestHelpers::Port(9002), NetMode);
    Executor->Then<FSpawnBeaconOnPIEChainedTask>(BeaconHost2, BeaconHostObject2, 1, TestHelpers::Port(9003), NetMode);

    Executor->Then<FConnectClientPIEToHostPIEBeaconChainedTask>(BeaconClientTo1, 0, 2, TestHelpers::Port(9002));
    Executor->Then<FConnectClientPIEToHostPIEBeaconChainedTask>(BeaconClientTo2, 1, 2, TestHelpers::Port(9003));

    Executor->Then<FCloseClientPIEBeaconChainedTask>(BeaconClientTo1);
    Executor->Then<FCloseClientPIEBeaconChainedTask>(BeaconClientTo2);

    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FConnectToBeaconsConcurrently_OverIP,
    "Redpoint.EOS.Networking.OverIP.ConnectToBeaconsConcurrently",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FConnectToBeaconsConcurrently_OverIP::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ConnectToBeaconsConcurrently_RunAsyncTest(this, TEXT("NetMode=ForceIP"), OnDone);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FConnectToBeaconsConcurrently_OverP2P,
    "Redpoint.EOS.Networking.OverP2P.ConnectToBeaconsConcurrently",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FConnectToBeaconsConcurrently_OverP2P::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ConnectToBeaconsConcurrently_RunAsyncTest(this, TEXT("NetMode=ForceP2P"), OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()