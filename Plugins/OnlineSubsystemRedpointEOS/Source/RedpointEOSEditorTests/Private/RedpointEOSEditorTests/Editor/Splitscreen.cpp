// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"
#include "RedpointEOSTests/TestUtilities/AutomationMacros.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2061704179, Redpoint::EOS::Editor::Tests::Editor)
{
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FEditor_Splitscreen,
    "Redpoint.EOS.Editor.Splitscreen",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

void FEditor_Splitscreen::RunAsyncTest(const TFunction<void()> &OnDone)
{
    TSharedRef<FEditorChainedTaskExecutor> Executor =
        MakeShareable(new FEditorChainedTaskExecutor(this, OnDone, TArray<int32>{2, 2}, EPlayNetMode::PIE_Standalone));

    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()