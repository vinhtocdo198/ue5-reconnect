// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSEditorTests/Networking/ConnectClientPIEToHostPIEChainedTask.h"
#include "RedpointEOSEditorTests/Networking/MakePIEStartListeningServerChainedTask.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1217569839, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Editor::Tests::Networking;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

static void ConnectAfterNetworkDisruption_RunAsyncTest(
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

    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            auto Platform = ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle();
            EOS_Platform_SetNetworkStatus(Platform->Handle(), EOS_ENetworkStatus::EOS_NS_Offline);
            Platform->OnNetworkStatusChanged().Broadcast(
                Platform.Get(),
                EOS_ENetworkStatus::EOS_NS_Online,
                EOS_ENetworkStatus::EOS_NS_Offline);
            EOS_Platform_SetNetworkStatus(Platform->Handle(), EOS_ENetworkStatus::EOS_NS_Online);
            Platform->OnNetworkStatusChanged().Broadcast(
                Platform.Get(),
                EOS_ENetworkStatus::EOS_NS_Offline,
                EOS_ENetworkStatus::EOS_NS_Online);
            TaskOnDone.ExecuteIfBound(true);
        });

    // @note: This needs a longer timeout to allow P2P subsystems to start up again after the network disruption and
    // make a successful connection.
    Executor->Then<FConnectClientPIEToHostPIEChainedTask>(0, 1, TestHelpers::Port(9000), FSimpleDelegate(), 60.0);

    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FConnectAfterNetworkDisruption_OverP2P,
    "Redpoint.EOS.Networking.OverP2P.ConnectAfterNetworkDisruption",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FConnectAfterNetworkDisruption_OverP2P::RunAsyncTest(const TFunction<void()> &OnDone)
{
    ConnectAfterNetworkDisruption_RunAsyncTest(this, TEXT("NetMode=ForceP2P"), OnDone);
}

}

REDPOINT_EOS_CODE_GUARD_END()