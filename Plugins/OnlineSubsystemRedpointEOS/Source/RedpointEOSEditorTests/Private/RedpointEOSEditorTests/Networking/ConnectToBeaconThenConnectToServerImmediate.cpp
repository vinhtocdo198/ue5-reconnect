// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEBeaconChainedTask.h"
#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEChainedTask.h"
#include "RedpointEOSEditorTests/Networking/MakePIEStartListeningServerChainedTask.h"
#include "RedpointEOSEditorTests/Networking/SpawnBeaconOnPIEChainedTask.h"
#include "RedpointEOSEditorTests/Networking/WaitForClientPIEBeaconPongChainedTask.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/DelayChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2571626866, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Editor::Tests::Networking;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Tests::TestUtilities;

static void ConnectToBeaconThenConnectToServerImmediate_RunAsyncTest(
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

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> BeaconHost =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> BeaconHostObject =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>>();

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClientTo =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>>();

    Executor->Then<FMakePIEStartListeningServerChainedTask>(0, TestHelpers::Port(9000), NetMode);

    Executor->Then<FSpawnBeaconOnPIEChainedTask>(BeaconHost, BeaconHostObject, 0, TestHelpers::Port(9001), NetMode);

    Executor->Then<FConnectClientPIEToHostPIEBeaconChainedTask>(BeaconClientTo, 0, 1, TestHelpers::Port(9001));
    Executor->Then<FLambdaChainedTask>(
        [BeaconClientTo](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconClientTo)->bClientGotPong = false;
            (*BeaconClientTo)->ServerPing();
            TaskOnDone.ExecuteIfBound(true);
        });
    Executor->Then<FWaitForClientPIEBeaconPongChainedTask>(BeaconClientTo);

    // @note: Don't use FCloseClientPIEBeaconChainedTask because we don't want to wait for the beacon to fully shutdown. Instead use the 'BeforeBrowse' delegate to do it immediately before the Browse call.

    Executor->Then<FConnectClientPIEToHostPIEChainedTask>(0, 1, TestHelpers::Port(9000), FSimpleDelegate::CreateLambda([BeaconClientTo]() {
        (*BeaconClientTo)->DestroyBeacon();
    }));

    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FConnectToBeaconThenConnectToServerImmediate_OverIP,
    "Redpoint.EOS.Networking.OverIP.ConnectToBeaconThenConnectToServerImmediate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FConnectToBeaconThenConnectToServerImmediate_OverIP::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ConnectToBeaconThenConnectToServerImmediate_RunAsyncTest(this, TEXT("NetMode=ForceIP"), OnDone);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FConnectToBeaconThenConnectToServerImmediate_OverP2P,
    "Redpoint.EOS.Networking.OverP2P.ConnectToBeaconThenConnectToServerImmediate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FConnectToBeaconThenConnectToServerImmediate_OverP2P::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ConnectToBeaconThenConnectToServerImmediate_RunAsyncTest(this, TEXT("NetMode=ForceP2P"), OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()