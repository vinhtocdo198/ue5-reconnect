// Copyright June Rhodes. All Rights Reserved.

#include "HAL/MemoryMisc.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSTestsModule.h"
#include "TestHelpers.h"
#include "Tests/AutomationCommon.h"

extern const char UC_TestFile_Small[];
extern const size_t UC_TestFile_SmallSize;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineUserCloudInterface_CanWriteThenReadFile,
    "Redpoint.EOS.OnlineUserCloudInterface.CanWriteThenReadFile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineUserCloudInterface_CanWriteThenReadFile::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FString TargetFilename = TEXT("TestFile.Small");

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
                    [this](uint32 BytesWritten, const FUniqueNetId &UserId, const FString &Filename) {
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
                        if (!bWasSuccessful || Filename != TargetFilename)
                        {
                            OnDone();
                            return;
                        }

                        auto CancelReadUserFile = RegisterOSSCallback(
                            this,
                            UserCloud,
                            &IOnlineUserCloud::AddOnReadUserFileCompleteDelegate_Handle,
                            &IOnlineUserCloud::ClearOnReadUserFileCompleteDelegate_Handle,
                            TFunction<void(bool, const FUniqueNetId &, const FString &)>(
                                [this,
                                 OnDone,
                                 TargetFilename,
                                 // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                 UserCloud](bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Filename) {
                                    TestEqual("Read filename matches expected value", Filename, TargetFilename);
                                    TestTrue("Read operation succeeded", bWasSuccessful);
                                    if (!bWasSuccessful || Filename != TargetFilename)
                                    {
                                        OnDone();
                                        return;
                                    }

                                    TArray<uint8> FileContentsRead;
                                    TestTrue(
                                        "Can call GetFileContents",
                                        UserCloud->GetFileContents(UserId, Filename, FileContentsRead));
                                    TestEqual(
                                        "File has expected length",
                                        FileContentsRead.Num(),
                                        UC_TestFile_SmallSize);
                                    if (FileContentsRead.Num() == UC_TestFile_SmallSize)
                                    {
                                        for (int i = 0; i < UC_TestFile_SmallSize; i++)
                                        {
                                            TestEqual(
                                                FString::Printf(TEXT("File byte at index %d matches"), i),
                                                FileContentsRead[i],
                                                UC_TestFile_Small[i]);
                                        }
                                    }
                                    OnDone();
                                }));

                        bool bStarted = UserCloud->ReadUserFile(UserId, TargetFilename);
                        TestTrue("ReadUserFile operation started", bStarted);
                        if (!bStarted)
                        {
                            CancelReadUserFile();
                            OnDone();
                        }
                    }));

            TArray<uint8> FileContents = TArray<uint8>((uint8 *)UC_TestFile_Small, UC_TestFile_SmallSize);
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