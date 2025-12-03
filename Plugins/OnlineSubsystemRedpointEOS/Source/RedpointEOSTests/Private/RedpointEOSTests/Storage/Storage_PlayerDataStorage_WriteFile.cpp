// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSStorage/StorageSystem.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(587025308, Redpoint::EOS::Tests::Storage)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Storage_PlayerDataStorage_WriteFile,
    "Redpoint.EOS.Storage.PlayerDataStorage.WriteFile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Storage_PlayerDataStorage_WriteFile::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Storage;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &TaskOnDone) {
        auto UC_TestFile_Small = TArray<uint8>{'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
        auto StorageSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IStorageSystem>();
        StorageSystem->Write(
            GetAccountId(Context->User(0).GetProductUserId()),
            TEXT("TestFile.Small.txt"),
            UC_TestFile_Small,
            IStorageSystem::FOnWriteComplete::CreateLambda([Context, TaskOnDone, UC_TestFile_Small, StorageSystem](
                                                               const FError &ErrorCode) {
                if (!Context->Assert()
                         .TestEqual(TEXT("Operation succeeded"), ErrorCode.ResultCode(), EOS_EResult::EOS_Success))
                {
                    TaskOnDone.ExecuteIfBound(false);
                    return;
                }

                StorageSystem->Read(
                    GetAccountId(Context->User(0).GetProductUserId()),
                    EStorageType::PlayerDataStorage,
                    TEXT("TestFile.Small.txt"),
                    IStorageSystem::FOnReadComplete::CreateLambda([Context, TaskOnDone, UC_TestFile_Small](
                                                                      const FError &ReadErrorCode,
                                                                      const TArray<uint8> &FileData) {
                        Context->Assert().TestEqual(
                            TEXT("Operation succeeded"),
                            ReadErrorCode.ResultCode(),
                            EOS_EResult::EOS_Success);
                        if (Context->Assert()
                                .TestEqual(TEXT("File has expected length"), FileData.Num(), UC_TestFile_Small.Num()))
                        {
                            for (int32 i = 0; i < UC_TestFile_Small.Num(); i++)
                            {
                                Context->Assert().TestEqual(
                                    FString::Printf(TEXT("File byte at index %d matches"), i),
                                    FileData[i],
                                    UC_TestFile_Small[i]);
                            }
                        }
                        TaskOnDone.ExecuteIfBound(true);
                    }));
            }));
    });

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()