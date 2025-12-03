// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSTests/TitleFile/ReadTitleFileChainedTask.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3029637292, Redpoint::EOS::Tests::TitleFile)
{
using namespace ::Redpoint::EOS::Tests::TitleFile;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_TitleFile_CanReadTitleFileDedicatedServer,
    "Redpoint.EOS.TitleFile.CanReadTitleFileDedicatedServer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_TitleFile_CanReadTitleFileDedicatedServer::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(
        new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}, EChainedTaskExecutorFlag::DedicatedServer));

    // Read and validate the file in Title Storage.
    Executor->Then<FReadTitleFileChainedTask>(0, TEXT("TestFile.Small.txt"));

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()