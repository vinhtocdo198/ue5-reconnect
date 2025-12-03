// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/TitleFile/OnlineTitleFileInterfaceEOS.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/TitleFile/PagedQueryEOS.h"
#include "RedpointEOSAPI/TitleStorage/DeleteCache.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSStorage/StorageSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(496971326, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile;
using namespace ::Redpoint::EOS::API::TitleStorage;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Storage;

FOnlineTitleFileInterfaceEOS::FOnlineTitleFileInterfaceEOS(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , ListCache()
    , DownloadCache()
{
}

TSharedRef<const FUniqueNetIdEOS> FOnlineTitleFileInterfaceEOS::GetLocalUserId() const
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(0);
    if (!IdentityUser.IsValid())
    {
        return FUniqueNetIdEOS::DedicatedServerId();
    }

    return GetUniqueNetIdRef(IdentityUser->GetUserId());
}

bool FOnlineTitleFileInterfaceEOS::GetFileContents(const FString &FileName, TArray<uint8> &FileContents)
{
    auto *File = this->DownloadCache.Find(FileName);
    if (File != nullptr)
    {
        FileContents = *File;
        return true;
    }
    return false;
}

bool FOnlineTitleFileInterfaceEOS::ClearFiles()
{
    this->DownloadCache.Empty();
    return true;
}

bool FOnlineTitleFileInterfaceEOS::ClearFile(const FString &FileName)
{
    if (this->DownloadCache.Contains(FileName))
    {
        this->DownloadCache.Remove(FileName);
        return true;
    }
    return false;
}

void FOnlineTitleFileInterfaceEOS::DeleteCachedFiles(bool bSkipEnumerated)
{
    FDeleteCache::Execute(
        this->PlatformHandle,
        FDeleteCache::Options{this->GetLocalUserId()->GetProductUserId()},
        FDeleteCache::CompletionDelegate::CreateSPLambda(this, [](const FDeleteCache::Result &Result) {
            if (Result.ResultCode != EOS_EResult::EOS_Success)
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Error,
                    TEXT("%s"),
                    *::Redpoint::EOS::API::ConvertError(
                         ANSI_TO_TCHAR(__FUNCTION__),
                         TEXT("Failed to clear Title File cache."),
                         Result.ResultCode)
                         .ToLogString());
            }
        }));
}

bool FOnlineTitleFileInterfaceEOS::EnumerateFiles(const FPagedQuery &Page)
{
    if (Page.Start != 0 || Page.Count != -1)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: Must provide default FPagedQuery with Start set to 0 and Count set to -1; EOS does not support "
                 "pagination for QueryFileList."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return false;
    }

    // If you want to customize the tags that are queryed over, pass in FPagedQueryEOS (located
    // in the "OnlineTitleFilePagedQueryEOS.h" header instead of the default FPagedQuery class.
    TArray<FString> Tags;
    if (FPagedQueryEOS::IsFPagedQueryEOS(Page))
    {
        Tags = ((const FPagedQueryEOS &)Page).Tags;
    }
    if (Tags.Num() == 0)
    {
        Tags.Add(TEXT("Default"));
    }

    auto LocalUserId = this->GetLocalUserId();

    auto StorageSystem = this->PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->List(
        LocalUserId->IsDedicatedServer() ? TOptional<UE::Online::FAccountId>() : GetAccountId(LocalUserId),
        EStorageType::TitleStorage,
        Tags,
        IStorageSystem::FOnListComplete::CreateSPLambda(
            this,
            [this](const FError &ErrorCode, const TArray<FFileEntry> &FileList) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("%s: %s"),
                        ANSI_TO_TCHAR(__FUNCTION__),
                        *ErrorCode.ToLogString());
                    this->TriggerOnEnumerateFilesCompleteDelegates(false, ErrorCode.ToLogString());
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
                    this->ListCache = Results;

                    this->TriggerOnEnumerateFilesCompleteDelegates(true, TEXT(""));
                }
            }));
    return true;
}

void FOnlineTitleFileInterfaceEOS::GetFileList(TArray<FCloudFileHeader> &UserFiles)
{
    UserFiles = this->ListCache;
}

bool FOnlineTitleFileInterfaceEOS::ReadFile(const FString &FileName)
{
    auto LocalUserId = this->GetLocalUserId();

    auto StorageSystem = this->PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->Read(
        LocalUserId->IsDedicatedServer() ? TOptional<UE::Online::FAccountId>() : GetAccountId(LocalUserId),
        EStorageType::TitleStorage,
        FileName,
        IStorageSystem::FOnReadComplete::CreateSPLambda(
            this,
            [this, FileName](const FError &ErrorCode, const TArray<uint8> &FileData) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    this->TriggerOnReadFileCompleteDelegates(false, FileName);
                    return;
                }

                this->DownloadCache.Add(FileName, FileData);
                this->TriggerOnReadFileCompleteDelegates(true, FileName);
            }),
        IStorageSystem::FOnOperationProgress::CreateSPLambda(
            this,
            [this, FileName](uint32 BytesTransferred, uint32 TotalFileSizeBytes) {
                this->TriggerOnReadFileProgressDelegates(FileName, BytesTransferred);
            }));
    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()