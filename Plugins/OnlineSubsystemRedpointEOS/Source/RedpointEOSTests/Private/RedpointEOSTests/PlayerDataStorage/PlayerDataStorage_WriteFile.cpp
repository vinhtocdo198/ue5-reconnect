// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSAPI/PlayerDataStorage/ReadFile.h"
#include "RedpointEOSAPI/PlayerDataStorage/WriteFile.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4100859373, Redpoint::EOS::Tests::PlayerDataStorage)
{

static const char UC_TestFile_Small[] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
static const int32 UC_TestFile_SmallSize = (int32)sizeof(UC_TestFile_Small);

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_PlayerDataStorage_WriteFile,
    "Redpoint.EOS.PlayerDataStorage.WriteFile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_PlayerDataStorage_WriteFile::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::API::PlayerDataStorage;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        auto Buffer =
            MakeShared<TArray<uint8>>(reinterpret_cast<const uint8 *>(UC_TestFile_Small), UC_TestFile_SmallSize);

        FWriteFile::Execute(
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle(),
            FWriteFile::Options{Context->User(0).GetProductUserId(), TEXT("TestFile.Small.txt"), 256},
            FWriteFile::WriteDelegate::CreateLambda(
                [Buffer](const FWriteFile::WriteResult &Result, TArray<uint8> &OutData) -> FWriteFile::EWriteResult {
                    OutData = *Buffer;
                    Buffer->Empty();
                    return FWriteFile::EWriteResult::CompleteRequest;
                }),
            FWriteFile::ProgressDelegate::CreateLambda([](const FWriteFile::ProgressResult &) {
            }),
            FWriteFile::CompletionDelegate::CreateLambda(
                [Context, Buffer, TaskOnDone](const FWriteFile::Result &WriteResult) {
                    if (Context->Assert()
                            .TestEqual(TEXT("Operation succeeded"), WriteResult.ResultCode, EOS_EResult::EOS_Success))
                    {
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
                            FReadFile::CompletionDelegate::CreateLambda(
                                [Context, Buffer, TaskOnDone](const FReadFile::Result &Result) {
                                    Context->Assert().TestEqual(
                                        TEXT("Operation succeeded"),
                                        Result.ResultCode,
                                        EOS_EResult::EOS_Success);
                                    if (Context->Assert().TestEqual(
                                            TEXT("File has expected length"),
                                            Buffer->Num(),
                                            UC_TestFile_SmallSize))
                                    {
                                        for (int32 i = 0; i < UC_TestFile_SmallSize; i++)
                                        {
                                            Context->Assert().TestEqual(
                                                FString::Printf(TEXT("File byte at index %d matches"), i),
                                                (*Buffer)[i],
                                                UC_TestFile_Small[i]);
                                        }
                                    }
                                    TaskOnDone.ExecuteIfBound(true);
                                }));
                    }
                    else
                    {
                        TaskOnDone.ExecuteIfBound(true);
                    }
                }));
    });

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()