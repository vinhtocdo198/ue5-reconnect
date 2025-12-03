// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSAPI/TitleStorage/ReadFile.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1284601540, Redpoint::EOS::Tests::TitleStorage)
{

static const char NS_TF_TestFile_Small[] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
static const int32 NS_TF_TestFile_SmallSize = (int32)sizeof(NS_TF_TestFile_Small);

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_TitleStorage_ReadFile,
    "Redpoint.EOS.TitleStorage.ReadFile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_TitleStorage_ReadFile::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::API::TitleStorage;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        auto Buffer = MakeShared<TArray<uint8>>();
        FReadFile::Execute(
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle(),
            FReadFile::Options{Context->User(0).GetProductUserId(), TEXT("TestFile.Small.txt"), 256},
            FReadFile::ReadDelegate::CreateLambda(
                [Buffer](const FReadFile::ReadResult &Result) -> FReadFile::EReadResult {
                    for (const auto &Byte : Result.DataChunk)
                    {
                        Buffer->Add(Byte);
                    }
                    return FReadFile::EReadResult::ContinueReading;
                }),
            FReadFile::ProgressDelegate::CreateLambda([](const FReadFile::ProgressResult &) {
            }),
            FReadFile::CompletionDelegate::CreateLambda([Context, Buffer, TaskOnDone](const FReadFile::Result &Result) {
                Context->Assert().TestEqual(TEXT("Operation succeeded"), Result.ResultCode, EOS_EResult::EOS_Success);
                if (Context->Assert()
                        .TestEqual(TEXT("File has expected length"), Buffer->Num(), NS_TF_TestFile_SmallSize))
                {
                    for (int32 i = 0; i < NS_TF_TestFile_SmallSize; i++)
                    {
                        Context->Assert().TestEqual(
                            FString::Printf(TEXT("File byte at index %d matches"), i),
                            (*Buffer)[i],
                            NS_TF_TestFile_Small[i]);
                    }
                }
                TaskOnDone.ExecuteIfBound(true);
            }));
    });

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()