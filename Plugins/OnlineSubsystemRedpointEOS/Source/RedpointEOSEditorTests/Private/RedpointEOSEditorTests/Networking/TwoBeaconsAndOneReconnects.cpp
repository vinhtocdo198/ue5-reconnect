// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSEditorTests/Networking/CloseClientPIEBeaconChainedTask.h"
#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEBeaconChainedTask.h"
#include "RedpointEOSEditorTests/Networking/MakePIEStartListeningServerChainedTask.h"
#include "RedpointEOSEditorTests/Networking/SpawnBeaconOnPIEChainedTask.h"
#include "RedpointEOSEditorTests/Networking/WaitForClientPIEBeaconPongChainedTask.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(468258783, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Editor::Tests::Networking;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Tests::TestUtilities;

static void TwoBeaconsAndOneReconnects_RunAsyncTest(
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

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> BeaconHost1 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> BeaconHostObject1 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClientTo1 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>>();

    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> BeaconHost2 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> BeaconHostObject2 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>>();
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClientTo2 =
        MakeShared<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>>();

    Executor->Then<FSpawnBeaconOnPIEChainedTask>(BeaconHost1, BeaconHostObject1, 0, TestHelpers::Port(9000), NetMode);
    Executor->Then<FSpawnBeaconOnPIEChainedTask>(BeaconHost2, BeaconHostObject2, 0, TestHelpers::Port(9001), NetMode);

    Executor->Then<FConnectClientPIEToHostPIEBeaconChainedTask>(BeaconClientTo1, 0, 1, TestHelpers::Port(9000));
    Executor->Then<FConnectClientPIEToHostPIEBeaconChainedTask>(BeaconClientTo2, 0, 1, TestHelpers::Port(9001));

    Executor->Then<FLambdaChainedTask>(
        [BeaconClientTo1](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconClientTo1)->bClientGotPong = false;
            (*BeaconClientTo1)->ServerPing();
            TaskOnDone.ExecuteIfBound(true);
        });
    Executor->Then<FWaitForClientPIEBeaconPongChainedTask>(BeaconClientTo1);

    Executor->Then<FLambdaChainedTask>(
        [BeaconClientTo2](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconClientTo2)->bClientGotPong = false;
            (*BeaconClientTo2)->ServerPing();
            TaskOnDone.ExecuteIfBound(true);
        });
    Executor->Then<FWaitForClientPIEBeaconPongChainedTask>(BeaconClientTo2);

    Executor->Then<FCloseClientPIEBeaconChainedTask>(BeaconClientTo2);

    Executor->Then<FLambdaChainedTask>(
        [BeaconClientTo2](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            BeaconClientTo2->Reset();
            FTSTicker::GetCoreTicker().AddTicker(
                FTickerDelegate::CreateLambda([TaskOnDone](float) {
                    TaskOnDone.ExecuteIfBound(true);
                    return false;
                }),
                2.0f);
        });

    Executor->Then<FSpawnBeaconOnPIEChainedTask>(BeaconHost2, BeaconHostObject2, 0, TestHelpers::Port(9001), NetMode);

    Executor->Then<FConnectClientPIEToHostPIEBeaconChainedTask>(BeaconClientTo2, 0, 1, TestHelpers::Port(9001));

    Executor->Then<FLambdaChainedTask>(
        [BeaconClientTo2](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconClientTo2)->bClientGotPong = false;
            (*BeaconClientTo2)->ServerPing();
            TaskOnDone.ExecuteIfBound(true);
        });
    Executor->Then<FWaitForClientPIEBeaconPongChainedTask>(BeaconClientTo2);

    Executor->Then<FLambdaChainedTask>(
        [BeaconClientTo1](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            (*BeaconClientTo1)->bClientGotPong = false;
            (*BeaconClientTo1)->ServerPing();
            TaskOnDone.ExecuteIfBound(true);
        });
    Executor->Then<FWaitForClientPIEBeaconPongChainedTask>(BeaconClientTo1);

    Executor->Then<FCloseClientPIEBeaconChainedTask>(BeaconClientTo1);
    Executor->Then<FCloseClientPIEBeaconChainedTask>(BeaconClientTo2);

    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FTwoBeaconsAndOneReconnects_OverIP,
    "Redpoint.EOS.Networking.OverIP.TwoBeaconsAndOneReconnects",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FTwoBeaconsAndOneReconnects_OverIP::RunAsyncTest(const TFunction<void()> &OnDone)
{
    TwoBeaconsAndOneReconnects_RunAsyncTest(this, TEXT("NetMode=ForceIP"), OnDone);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FTwoBeaconsAndOneReconnects_OverP2P,
    "Redpoint.EOS.Networking.OverP2P.TwoBeaconsAndOneReconnects",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FTwoBeaconsAndOneReconnects_OverP2P::RunAsyncTest(const TFunction<void()> &OnDone)
{
    TwoBeaconsAndOneReconnects_RunAsyncTest(this, TEXT("NetMode=ForceP2P"), OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()