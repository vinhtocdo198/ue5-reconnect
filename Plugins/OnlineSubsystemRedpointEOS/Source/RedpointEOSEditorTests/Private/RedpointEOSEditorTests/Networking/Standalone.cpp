// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1297468799, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FStandalone_OverIP,
    "Redpoint.EOS.Networking.Standalone",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FStandalone_OverIP::RunAsyncTest(const TFunction<void()> &OnDone)
{
    TSharedRef<FEditorChainedTaskExecutor> Executor = MakeShareable(new FEditorChainedTaskExecutor(
        this,
        OnDone,
        TArray<int32>{1},
        EPlayNetMode::PIE_Standalone,
        TEXT("?NetMode=ForceIP")));

    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()