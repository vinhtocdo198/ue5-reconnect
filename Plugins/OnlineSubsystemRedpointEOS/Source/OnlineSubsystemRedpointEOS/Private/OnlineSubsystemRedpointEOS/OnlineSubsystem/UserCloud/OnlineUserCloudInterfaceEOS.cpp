// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/UserCloud/OnlineUserCloudInterfaceEOS.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/SubsystemNames.h"
#include "RedpointEOSCore/Id/Id.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(513699043, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::UserCloud)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem;
using namespace ::Redpoint::EOS::Storage;

FOnlineUserCloudInterfaceEOS::FOnlineUserCloudInterfaceEOS(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , ListCache()
    , DownloadCache()
{
}

bool FOnlineUserCloudInterfaceEOS::GetFileContents(
    const FUniqueNetId &UserId,
    const FString &FileName,
    TArray<uint8> &FileContents)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return false;
    }

    auto *Cache = this->DownloadCache.Find(UserId.AsShared());
    if (Cache != nullptr)
    {
        auto *File = Cache->Find(FileName);
        if (File != nullptr)
        {
            FileContents = *File;
            return true;
        }
    }
    return false;
}

bool FOnlineUserCloudInterfaceEOS::ClearFiles(const FUniqueNetId &UserId)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return false;
    }

    this->DownloadCache.Remove(UserId);
    return true;
}

bool FOnlineUserCloudInterfaceEOS::ClearFile(const FUniqueNetId &UserId, const FString &FileName)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return false;
    }

    auto *Cache = this->DownloadCache.Find(UserId.AsShared());
    if (Cache != nullptr)
    {
        if (Cache->Contains(FileName))
        {
            Cache->Remove(FileName);
            return true;
        }
    }
    return false;
}

void FOnlineUserCloudInterfaceEOS::EnumerateUserFiles(const FUniqueNetId &UserId)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        this->TriggerOnEnumerateUserFilesCompleteDelegates(false, UserId);
        return;
    }

    auto StorageSystem = this->PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->List(
        GetAccountId(UserId),
        EStorageType::PlayerDataStorage,
        TArray<FString>(),
        IStorageSystem::FOnListComplete::CreateSPLambda(
            this,
            [this, AccountId = GetAccountId(UserId)](const FError &ErrorCode, const TArray<FFileEntry> &FileList) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("%s: %s"),
                        ANSI_TO_TCHAR(__FUNCTION__),
                        *ErrorCode.ToLogString());
                    this->TriggerOnEnumerateUserFilesCompleteDelegates(false, *GetUniqueNetIdRef(AccountId));
                }
                else
                {
                    TArray<FCloudFileHeader> Results;
                    for (const auto &FileEntry : FileList)
                    {
                        auto Header = FCloudFileHeader(
                            FileEntry.FilePath,
                            FileEntry.FilePath,
                            static_cast<int32>(FileEntry.FileSizeBytes));
                        Header.Hash = FileEntry.MD5Hash;
                        Header.HashType = FName(TEXT("MD5"));
                        Header.URL = TEXT("");
                        Header.ChunkID = 0;
                        Header.ExternalStorageIds = TMap<FString, FString>();
                        Results.Add(Header);
                    }
                    this->ListCache.Add(*GetUniqueNetIdRef(AccountId), Results);

                    this->TriggerOnEnumerateUserFilesCompleteDelegates(true, *GetUniqueNetIdRef(AccountId));
                }
            }));
}

void FOnlineUserCloudInterfaceEOS::GetUserFileList(const FUniqueNetId &UserId, TArray<FCloudFileHeader> &UserFiles)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    auto *Cache = this->ListCache.Find(UserId.AsShared());
    if (Cache != nullptr)
    {
        UserFiles = *Cache;
    }
    else
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: No file list was cached for this user yet; call EnumerateUserFiles first."),
            ANSI_TO_TCHAR(__FUNCTION__));
    }
}

bool FOnlineUserCloudInterfaceEOS::ReadUserFile(const FUniqueNetId &UserId, const FString &FileName)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return false;
    }

    auto StorageSystem = this->PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->Read(
        GetAccountId(UserId),
        EStorageType::PlayerDataStorage,
        FileName,
        IStorageSystem::FOnReadComplete::CreateSPLambda(
            this,
            [this, FileName, AccountId = GetAccountId(UserId)](const FError &ErrorCode, const TArray<uint8> &FileData) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    this->TriggerOnReadUserFileCompleteDelegates(false, *GetUniqueNetIdRef(AccountId), FileName);
                    return;
                }

                auto &Cache = this->DownloadCache.FindOrAdd(GetUniqueNetIdRef(AccountId));
                Cache.Add(FileName, FileData);
                this->TriggerOnReadUserFileCompleteDelegates(true, *GetUniqueNetIdRef(AccountId), FileName);
            }));
    return true;
}

bool FOnlineUserCloudInterfaceEOS::WriteUserFile(
    const FUniqueNetId &UserId,
    const FString &FileName,
    TArray<uint8> &FileContents,
    bool bCompressBeforeUpload)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return false;
    }

    if (bCompressBeforeUpload)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Warning,
            TEXT("%s: bCompressBeforeUpload is not supported; file will not be compressed."),
            ANSI_TO_TCHAR(__FUNCTION__));
    }

    auto StorageSystem = this->PlatformHandle->GetSystem<IStorageSystem>();
    this->WriteOperations.FindOrAdd(UserId.AsShared())
        .Add(
            FileName,
            StorageSystem->Write(
                GetAccountId(UserId),
                FileName,
                FileContents,
                IStorageSystem::FOnWriteComplete::CreateSPLambda(
                    this,
                    [this, FileName, AccountId = GetAccountId(UserId)](const FError &ErrorCode) {
                        if (!ErrorCode.WasSuccessful())
                        {
                            UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                            this->TriggerOnWriteUserFileCompleteDelegates(
                                false,
                                *GetUniqueNetIdRef(AccountId),
                                FileName);
                            return;
                        }

                        this->TriggerOnWriteUserFileCompleteDelegates(true, *GetUniqueNetIdRef(AccountId), FileName);
                    }),
                IStorageSystem::FOnOperationProgress::CreateSPLambda(
                    this,
                    [this,
                     FileName,
                     AccountId = GetAccountId(UserId)](uint32 BytesTransferred, uint32 TotalFileSizeBytes) {
#if !REDPOINT_EOS_UE_5_5_OR_LATER
                        PRAGMA_DISABLE_DEPRECATION_WARNINGS
                        this->TriggerOnWriteUserFileProgressDelegates(
                            static_cast<int32>(BytesTransferred),
                            *GetUniqueNetIdRef(AccountId),
                            FileName);
                        PRAGMA_ENABLE_DEPRECATION_WARNINGS
#endif
#if REDPOINT_EOS_UE_5_4_OR_LATER
                        this->TriggerOnWriteUserFileProgress64Delegates(
                            BytesTransferred,
                            *GetUniqueNetIdRef(AccountId),
                            FileName);
#endif
                    })));
    return true;
}

void FOnlineUserCloudInterfaceEOS::CancelWriteUserFile(const FUniqueNetId &UserId, const FString &FileName)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        this->TriggerOnWriteUserFileCanceledDelegates(false, UserId, FileName);
        return;
    }

    auto *OperationList = this->WriteOperations.Find(UserId.AsShared());
    if (OperationList == nullptr)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: This user does not have any write operations running."),
            ANSI_TO_TCHAR(__FUNCTION__));
        this->TriggerOnWriteUserFileCanceledDelegates(false, UserId, FileName);
        return;
    }

    auto *Operation = OperationList->Find(FileName);
    if (Operation == nullptr)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: No write operation has been started for the specified file."),
            ANSI_TO_TCHAR(__FUNCTION__));
        this->TriggerOnWriteUserFileCanceledDelegates(false, UserId, FileName);
        return;
    }

    auto StorageSystem = this->PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->CancelOperation(*Operation);

    this->TriggerOnWriteUserFileCanceledDelegates(true, UserId, FileName);
}

bool FOnlineUserCloudInterfaceEOS::DeleteUserFile(
    const FUniqueNetId &UserId,
    const FString &FileName,
    bool bShouldCloudDelete,
    bool bShouldLocallyDelete)
{
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: User ID was not a Redpoint EOS user ID."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return false;
    }

    if (!bShouldCloudDelete)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Warning,
            TEXT("%s: Setting bShouldCloudDelete to false has no effect; all files are stored in Player Data Storage."),
            ANSI_TO_TCHAR(__FUNCTION__));
    }

    if (bShouldLocallyDelete)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Warning,
            TEXT(
                "%s: Setting bShouldLocallyDelete to true has no effect; all files are stored in Player Data Storage."),
            ANSI_TO_TCHAR(__FUNCTION__));
    }

    auto StorageSystem = this->PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->Delete(
        GetAccountId(UserId),
        FileName,
        IStorageSystem::FOnDeleteComplete::CreateSPLambda(
            this,
            [this, FileName, AccountId = GetAccountId(UserId)](const FError &ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    this->TriggerOnDeleteUserFileCompleteDelegates(false, *GetUniqueNetIdRef(AccountId), FileName);
                    return;
                }

                this->TriggerOnDeleteUserFileCompleteDelegates(true, *GetUniqueNetIdRef(AccountId), FileName);
            }));
    return true;
}

bool FOnlineUserCloudInterfaceEOS::RequestUsageInfo(const FUniqueNetId &UserId)
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Error,
        TEXT("%s: Epic Online Services does not support reporting usage information about Player Data Storage."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return false;
}

void FOnlineUserCloudInterfaceEOS::DumpCloudState(const FUniqueNetId &UserId)
{
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s: Not supported."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FOnlineUserCloudInterfaceEOS::DumpCloudFileState(const FUniqueNetId &UserId, const FString &FileName)
{
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s: Not supported."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FOnlineUserCloudInterfaceEOS::ReadUserFileToCache(
    const UE::Online::FAccountId &AccountId,
    const FString &FileName,
    const IStorageSystem::FOnReadComplete &OnComplete)
{
    auto StorageSystem = this->PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->Read(
        AccountId,
        EStorageType::PlayerDataStorage,
        FileName,
        IStorageSystem::FOnReadComplete::CreateSPLambda(
            this,
            [this, FileName, AccountId, OnComplete](const FError &ErrorCode, const TArray<uint8> &FileData) {
                if (ErrorCode.WasSuccessful())
                {
                    auto &Cache = this->DownloadCache.FindOrAdd(GetUniqueNetIdRef(AccountId));
                    Cache.Add(FileName, FileData);
                }

                OnComplete.ExecuteIfBound(ErrorCode, FileData);
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()