// Copyright June Rhodes. All Rights Reserved.

#include "HAL/MemoryMisc.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSTestsModule.h"
#include "TestHelpers.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineUserCloudInterface_CanWriteLargeFile,
    "Redpoint.EOS.OnlineUserCloudInterface.CanWriteLargeFile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineUserCloudInterface_CanWriteLargeFile::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FString TargetFilename = TEXT("TestFile.Large");

    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this, TargetFilename](
            const IOnlineSubsystemPtr &Subsystem,
            const TSharedPtr<const FUniqueNetIdEOS> &UserId,
            const FOnDone &OnDone) {
            if (!(Subsystem.IsValid() && UserId.IsValid()))
            {
                this->AddError(FString::Printf(TEXT("Unable to init subsystem / authenticate")));
                OnDone();
                return;
            }

            auto UserCloud = Subsystem->GetUserCloudInterface();
            TestTrue("Online subsystem provides IOnlineUserCloud interface", UserCloud.IsValid());

            if (!UserCloud.IsValid())
            {
                OnDone();
                return;
            }

#if REDPOINT_EOS_UE_5_4_OR_LATER
            FDelegateHandle WriteProgressHandle =
                UserCloud->AddOnWriteUserFileProgress64Delegate_Handle(FOnWriteUserFileProgress64Delegate::CreateLambda(
                    [this](uint64 BytesWritten, const FUniqueNetId &UserId, const FString &Filename) {
                        this->AddInfo(FString::Printf(
                            TEXT("WriteUserFile progress, %llu bytes written for filename %s"),
                            BytesWritten,
                            *Filename));
                    }));
#else
            FDelegateHandle WriteProgressHandle =
                UserCloud->AddOnWriteUserFileProgressDelegate_Handle(FOnWriteUserFileProgressDelegate::CreateLambda(
                    [this](int32 BytesWritten, const FUniqueNetId &UserId, const FString &Filename) {
                        this->AddInfo(FString::Printf(
                            TEXT("WriteUserFile progress, %d bytes written for filename %s"),
                            BytesWritten,
                            *Filename));
                    }));
#endif

            auto CancelWriteUserFile = RegisterOSSCallback(
                this,
                UserCloud,
                &IOnlineUserCloud::AddOnWriteUserFileCompleteDelegate_Handle,
                &IOnlineUserCloud::ClearOnWriteUserFileCompleteDelegate_Handle,
                TFunction<void(bool, const FUniqueNetId &, const FString &)>(
                    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                    [this, OnDone, UserCloud, TargetFilename, WriteProgressHandle](
                        bool bWasSuccessful,
                        const FUniqueNetId &UserId,
                        const FString &Filename) {
                        FDelegateHandle WriteProgressHandleCopy = WriteProgressHandle;
#if REDPOINT_EOS_UE_5_4_OR_LATER
                        UserCloud->ClearOnWriteUserFileProgress64Delegate_Handle(WriteProgressHandleCopy);
#else
                        UserCloud->ClearOnWriteUserFileProgressDelegate_Handle(WriteProgressHandleCopy);
#endif

                        TestEqual("Write filename matches expected value", Filename, TargetFilename);
                        TestTrue("Write operation succeeded", bWasSuccessful);
                        OnDone();
                    }));

            // Allocate 170kb of data for the write.
            const size_t FileSize = 174080;
            uint8 *FileContentsRaw = (uint8 *)FMemory::Malloc(FileSize);
            for (size_t i = 0; i < FileSize; i++)
            {
                FileContentsRaw[i] = (uint8)('a' + (i % 4));
            }
            TArray<uint8> FileContents(FileContentsRaw, FileSize);
            FMemory::Free(FileContentsRaw);
            FileContentsRaw = nullptr;

            bool bStarted = UserCloud->WriteUserFile(*UserId.Get(), TargetFilename, FileContents);
            TestTrue("WriteUserFile operation started", bStarted);
            if (!bStarted)
            {
#if REDPOINT_EOS_UE_5_4_OR_LATER
                UserCloud->ClearOnWriteUserFileProgress64Delegate_Handle(WriteProgressHandle);
#else
                UserCloud->ClearOnWriteUserFileProgressDelegate_Handle(WriteProgressHandle);
#endif
                CancelWriteUserFile();
                OnDone();
            }
        });
}