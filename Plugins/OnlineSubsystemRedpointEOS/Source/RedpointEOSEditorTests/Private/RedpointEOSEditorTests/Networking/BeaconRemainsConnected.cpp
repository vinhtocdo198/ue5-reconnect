// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEBeaconChainedTask.h"
#include "RedpointEOSEditorTests/Networking/MakePIEStartListeningServerChainedTask.h"
#include "RedpointEOSEditorTests/Networking/SpawnBeaconOnPIEChainedTask.h"
#include "RedpointEOSEditorTests/Networking/WaitForClientPIEBeaconPongChainedTask.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/DelayChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(219165632, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Editor::Tests::Networking;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Tests::TestUtilities;

static void BeaconRemainsConnected_RunAsyncTest(
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

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> BeaconHost =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> BeaconHostObject =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>>();

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClientTo1 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClientTo2 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>>();

    TSharedRef<TSoftObjectPtr<AOnlineBeaconClient>> BeaconClientOnHost1 =
        MakeShared<TSoftObjectPtr<AOnlineBeaconClient>>();
    TSharedRef<TSoftObjectPtr<AOnlineBeaconClient>> BeaconClientOnHost2 =
        MakeShared<TSoftObjectPtr<AOnlineBeaconClient>>();

    Executor->Then<FSpawnBeaconOnPIEChainedTask>(BeaconHost, BeaconHostObject, 0, TestHelpers::Port(9000), NetMode);

    Executor->Then<FConnectClientPIEToHostPIEBeaconChainedTask>(BeaconClientTo1, 0, 1, TestHelpers::Port(9000));
    Executor->Then<FLambdaChainedTask>(
        [BeaconHostObject,
         BeaconClientOnHost1](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            *BeaconClientOnHost1 = (*BeaconHostObject)->GetClientActors()[0];
            TaskOnDone.ExecuteIfBound(true);
        });

    Executor->Then<FConnectClientPIEToHostPIEBeaconChainedTask>(BeaconClientTo2, 0, 2, TestHelpers::Port(9000));
    Executor->Then<FLambdaChainedTask>([BeaconHostObject, BeaconClientOnHost1, BeaconClientOnHost2](
                                           const FChainedTaskContextRef &,
                                           const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        for (const auto &Actor : (*BeaconHostObject)->GetClientActors())
        {
            if (*BeaconClientOnHost1 != Actor)
            {
                *BeaconClientOnHost2 = Actor;
                break;
            }
        }
        TaskOnDone.ExecuteIfBound(true);
    });

    Executor->Then<FLambdaChainedTask>(
        [BeaconClientTo1](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconClientTo1)->bClientGotPong = false;
            (*BeaconClientTo1)->ServerPing();
            TaskOnDone.ExecuteIfBound(true);
        });
    Executor->Then<FWaitForClientPIEBeaconPongChainedTask>(BeaconClientTo1);

    Executor->Then<FLambdaChainedTask>(
        [BeaconHost,
         BeaconClientOnHost1](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconHost)->DisconnectClient(BeaconClientOnHost1->Get());
            TaskOnDone.ExecuteIfBound(true);
        });

    // We are testing to make sure that closing client 1 doesn't cause client 2 to also be disconnected. Wait a little
    // for network connections to settle after calling DisconnectClient.
    Executor->Then<FDelayChainedTask>(3.0f);

    Executor->Then<FLambdaChainedTask>(
        [BeaconClientTo2](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconClientTo2)->bClientGotPong = false;
            (*BeaconClientTo2)->ServerPing();
            TaskOnDone.ExecuteIfBound(true);
        });
    Executor->Then<FWaitForClientPIEBeaconPongChainedTask>(BeaconClientTo2);

    Executor->Then<FLambdaChainedTask>(
        [BeaconHost,
         BeaconClientOnHost2](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconHost)->DisconnectClient(BeaconClientOnHost2->Get());
            TaskOnDone.ExecuteIfBound(true);
        });

    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FBeaconRemainsConnected_OverIP,
    "Redpoint.EOS.Networking.OverIP.BeaconRemainsConnected",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FBeaconRemainsConnected_OverIP::RunAsyncTest(const TFunction<void()> &OnDone)
{
    BeaconRemainsConnected_RunAsyncTest(this, TEXT("NetMode=ForceIP"), OnDone);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FBeaconRemainsConnected_OverP2P,
    "Redpoint.EOS.Networking.OverP2P.BeaconRemainsConnected",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FBeaconRemainsConnected_OverP2P::RunAsyncTest(const TFunction<void()> &OnDone)
{
    BeaconRemainsConnected_RunAsyncTest(this, TEXT("NetMode=ForceP2P"), OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()