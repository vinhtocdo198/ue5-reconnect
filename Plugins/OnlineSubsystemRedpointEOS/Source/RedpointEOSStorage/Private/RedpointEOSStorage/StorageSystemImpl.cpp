// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSStorage/StorageSystemImpl.h"

#include "RedpointEOSAPI/PlayerDataStorage/DeleteFile.h"
#include "RedpointEOSAPI/PlayerDataStorage/DuplicateFile.h"
#include "RedpointEOSAPI/PlayerDataStorage/QueryFileList.h"
#include "RedpointEOSAPI/PlayerDataStorage/ReadFile.h"
#include "RedpointEOSAPI/PlayerDataStorage/WriteFile.h"
#include "RedpointEOSAPI/PlayerDataStorageFileTransferRequest/CancelRequest.h"
#include "RedpointEOSAPI/TitleStorage/QueryFileList.h"
#include "RedpointEOSAPI/TitleStorage/ReadFile.h"
#include "RedpointEOSAPI/TitleStorageFileTransferRequest/CancelRequest.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSStorage/Logging.h"
#include "RedpointEOSStorage/StorageOperationImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOS_STORAGE_CHUNK_SIZE (16 * 1024)

namespace REDPOINT_EOS_FILE_NS_ID(2118794800, Redpoint::EOS::Storage)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Storage;

FStorageSystemImpl::FStorageSystemImpl(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
{
}

void FStorageSystemImpl::List(
    const TOptional<UE::Online::FAccountId> &LocalUserId,
    EStorageType StorageType,
    const TArray<FString> &TitleStorageTags,
    const FOnListComplete &OnComplete)
{
    TFunction<bool(const FString &)> Filter = [](const FString &FilePath) -> bool {
        // @note: We exclude files under __EOS__/ and __RedpointEOS__/ from appearing in list results to ensure the
        // plugin doesn't interfere with game code that is listing files.
        return !(FilePath.StartsWith("__EOS__/") || FilePath.StartsWith("__RedpointEOS__/"));
    };

    // Specifying "__AllowRedpointInternalFiles__" as a magic tag (even for Player Data Storage) makes the List
    // operation return internal files. This tag should only be specified for internal plugin code.
    auto RealTags = TitleStorageTags;
    if (RealTags.Contains(TEXT("__AllowRedpointInternalFiles__")))
    {
        RealTags.Remove(TEXT("__AllowRedpointInternalFiles__"));
        Filter = [](const FString &) -> bool {
            return true;
        };
    }

    switch (StorageType)
    {
    case EStorageType::PlayerDataStorage: {
        using namespace ::Redpoint::EOS::API::PlayerDataStorage;
        if (!LocalUserId.IsSet())
        {
            OnComplete.ExecuteIfBound(
                Errors::InvalidUser(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("A local user ID is required when accessing Player Data Storage.")),
                TArray<FFileEntry>());
            return;
        }
        FQueryFileList::Execute(
            this->PlatformHandle,
            FQueryFileList::Options{GetProductUserId(LocalUserId.GetValue())},
            FQueryFileList::CompletionDelegate::CreateSPLambda(
                this,
                [Filter, OnComplete](EOS_EResult ResultCode, const TArray<FQueryFileList::ResultEntry> &ResultEntries) {
                    TArray<FFileEntry> FileEntries;
                    for (const auto &Entry : ResultEntries)
                    {
                        if (Filter(Entry.Filename))
                        {
                            FileEntries.Add(FFileEntry{
                                Entry.FileSizeBytes,
                                Entry.MD5Hash,
                                Entry.Filename,
                                Entry.LastModifiedTime,
                                Entry.UnencryptedDataSizeBytes,
                            });
                        }
                    }
                    OnComplete.ExecuteIfBound(ConvertError(ResultCode), FileEntries);
                }));
        break;
    }
    case EStorageType::TitleStorage: {
        using namespace ::Redpoint::EOS::API::TitleStorage;
        FQueryFileList::Execute(
            this->PlatformHandle,
            FQueryFileList::Options{
                LocalUserId.IsSet() ? GetProductUserId(LocalUserId.GetValue()) : nullptr,
                RealTags.IsEmpty() ? TArray<FString>{TEXT("Default")} : RealTags},
            FQueryFileList::CompletionDelegate::CreateSPLambda(
                this,
                [Filter, OnComplete](EOS_EResult ResultCode, const TArray<FQueryFileList::ResultEntry> &ResultEntries) {
                    TArray<FFileEntry> FileEntries;
                    for (const auto &Entry : ResultEntries)
                    {
                        if (Filter(Entry.Filename))
                        {
                            FileEntries.Add(FFileEntry{
                                Entry.FileSizeBytes,
                                Entry.MD5Hash,
                                Entry.Filename,
                                TOptional<FDateTime>(),
                                Entry.UnencryptedDataSizeBytes,
                            });
                        }
                    }
                    OnComplete.ExecuteIfBound(ConvertError(ResultCode), FileEntries);
                }));
        break;
    }
    default: {
        OnComplete.ExecuteIfBound(
            Errors::NotImplemented(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Unhandled EStorageType value.")),
            TArray<FFileEntry>());
        break;
    }
    }
}

FStorageOperationRef FStorageSystemImpl::Read(
    const TOptional<UE::Online::FAccountId> &LocalUserId,
    EStorageType StorageType,
    const FString &FilePath,
    const FOnReadComplete &OnComplete,
    const FOnOperationProgress &OnProgress)
{
    auto AssembledBuffer = MakeShared<TArray<uint8>>();
    auto CurrentPosition = MakeShared<int32>();
    return this->ReadStream(
        LocalUserId,
        StorageType,
        FilePath,
        IStorageSystem::FOnReadChunk::CreateSPLambda(
            this,
            [AssembledBuffer,
             CurrentPosition](const TArray<uint8> &ChunkData, uint32 TotalFileSizeBytes, bool bIsLastChunk)
                -> EReadStreamAction {
                if (AssembledBuffer->Num() == 0)
                {
                    checkf(
                        TotalFileSizeBytes <= INT32_MAX,
                        TEXT("File for reading exceeds maximum supported file size for non-streamed read (%u > %d)"),
                        TotalFileSizeBytes,
                        INT32_MAX);
                    AssembledBuffer->SetNum(static_cast<int32>(TotalFileSizeBytes));
                }
                checkf(
                    *CurrentPosition + ChunkData.Num() <= AssembledBuffer->Num(),
                    TEXT("Chunk size plus position exceeds total file size bytes."));
                FMemory::Memcpy(
                    AssembledBuffer->GetData() + *CurrentPosition,
                    ChunkData.GetData(),
                    static_cast<SIZE_T>(ChunkData.Num()));
                *CurrentPosition += ChunkData.Num();
                return EReadStreamAction::Continue;
            }),
        IStorageSystem::FOnReadStreamComplete::CreateSPLambda(
            this,
            [OnComplete, AssembledBuffer](const FError &ErrorCode) {
                OnComplete.ExecuteIfBound(ErrorCode, *AssembledBuffer);
            }),
        OnProgress);
}

FStorageOperationRef FStorageSystemImpl::Write(
    const UE::Online::FAccountId &LocalUserId,
    const FString &FilePath,
    const TArray<uint8> &FileData,
    const FOnWriteComplete &OnComplete,
    const FOnOperationProgress &OnProgress)
{
    return this->WriteStream(
        LocalUserId,
        FilePath,
        IStorageSystem::FOnWriteChunk::CreateSPLambda(
            this,
            [FileData](TArray<uint8> &ChunkData) -> EWriteStreamAction {
                // @note: WriteStream will buffer the overflow here.
                ChunkData = FileData;
                return EWriteStreamAction::Complete;
            }),
        IStorageSystem::FOnWriteStreamComplete::CreateSPLambda(
            this,
            [OnComplete](const FError &ErrorCode) {
                OnComplete.ExecuteIfBound(ErrorCode);
            }),
        OnProgress);
}

void FStorageSystemImpl::Delete(
    const UE::Online::FAccountId &LocalUserId,
    const FString &FilePath,
    const FOnDeleteComplete &OnComplete)
{
    using namespace ::Redpoint::EOS::API::PlayerDataStorage;

    FDeleteFile::Execute(
        this->PlatformHandle,
        FDeleteFile::Options{GetProductUserId(LocalUserId), FilePath},
        FDeleteFile::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FDeleteFile::Result &DeleteResult) {
            OnComplete.ExecuteIfBound(ConvertError(DeleteResult.ResultCode));
        }));
}

class FStorageOperationPointerHolding
{
public:
    TSharedPtr<IStorageOperation> Operation;
};

FStorageOperationRef FStorageSystemImpl::ReadStream(
    const TOptional<UE::Online::FAccountId> &LocalUserId,
    EStorageType StorageType,
    const FString &FilePath,
    const FOnReadChunk &OnChunk,
    const FOnReadStreamComplete &OnComplete,
    const FOnOperationProgress &OnProgress)
{
    switch (StorageType)
    {
    case EStorageType::PlayerDataStorage: {
        using namespace ::Redpoint::EOS::API::PlayerDataStorage;
        if (!LocalUserId.IsSet())
        {
            OnComplete.ExecuteIfBound(Errors::InvalidUser(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("A local user ID is required when accessing Player Data Storage.")));
            return MakeShared<FStorageOperationImpl>();
        }
        auto Ptr = MakeShared<FStorageOperationPointerHolding>();
        Ptr->Operation = MakeShared<FStorageOperationImpl>(FReadFile::Execute(
            this->PlatformHandle,
            FReadFile::Options{GetProductUserId(LocalUserId.GetValue()), FilePath, REDPOINT_EOS_STORAGE_CHUNK_SIZE},
            FReadFile::ReadDelegate::CreateSPLambda(
                this,
                [Ptr, OnChunk](const FReadFile::ReadResult &Result) -> FReadFile::EReadResult {
                    if (StaticCastSharedPtr<FStorageOperationImpl>(Ptr->Operation)->bCancelled)
                    {
                        return FReadFile::EReadResult::CancelRequest;
                    }
                    else if (OnChunk.IsBound())
                    {
                        auto Action = OnChunk.Execute(Result.DataChunk, Result.TotalFileSizeBytes, Result.bIsLastChunk);
                        switch (Action)
                        {
                        case EReadStreamAction::Continue:
                            return FReadFile::EReadResult::ContinueReading;
                        case EReadStreamAction::Fail:
                            return FReadFile::EReadResult::FailRequest;
                        case EReadStreamAction::Cancel:
                            return FReadFile::EReadResult::CancelRequest;
                        default:
                            return FReadFile::EReadResult::FailRequest;
                        }
                    }
                    else
                    {
                        return FReadFile::EReadResult::FailRequest;
                    }
                }),
            FReadFile::ProgressDelegate::CreateSPLambda(
                this,
                [OnProgress](const FReadFile::ProgressResult &Result) {
                    OnProgress.ExecuteIfBound(Result.BytesTransferred, Result.TotalFileSizeBytes);
                }),
            FReadFile::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FReadFile::Result &Result) {
                OnComplete.ExecuteIfBound(ConvertError(Result.ResultCode));
            })));
        return Ptr->Operation.ToSharedRef();
    }
    case EStorageType::TitleStorage: {
        using namespace ::Redpoint::EOS::API::TitleStorage;
        auto Ptr = MakeShared<FStorageOperationPointerHolding>();
        Ptr->Operation = MakeShared<FStorageOperationImpl>(FReadFile::Execute(
            this->PlatformHandle,
            FReadFile::Options{
                LocalUserId.IsSet() ? GetProductUserId(LocalUserId.GetValue()) : nullptr,
                FilePath,
                REDPOINT_EOS_STORAGE_CHUNK_SIZE},
            FReadFile::ReadDelegate::CreateSPLambda(
                this,
                [Ptr, OnChunk](const FReadFile::ReadResult &Result) -> FReadFile::EReadResult {
                    if (StaticCastSharedPtr<FStorageOperationImpl>(Ptr->Operation)->bCancelled)
                    {
                        return FReadFile::EReadResult::CancelRequest;
                    }
                    else if (OnChunk.IsBound())
                    {
                        auto Action = OnChunk.Execute(Result.DataChunk, Result.TotalFileSizeBytes, Result.bIsLastChunk);
                        switch (Action)
                        {
                        case EReadStreamAction::Continue:
                            return FReadFile::EReadResult::ContinueReading;
                        case EReadStreamAction::Fail:
                            return FReadFile::EReadResult::FailRequest;
                        case EReadStreamAction::Cancel:
                            return FReadFile::EReadResult::CancelRequest;
                        default:
                            return FReadFile::EReadResult::FailRequest;
                        }
                    }
                    else
                    {
                        return FReadFile::EReadResult::FailRequest;
                    }
                }),
            FReadFile::ProgressDelegate::CreateSPLambda(
                this,
                [OnProgress](const FReadFile::ProgressResult &Result) {
                    OnProgress.ExecuteIfBound(Result.BytesTransferred, Result.TotalFileSizeBytes);
                }),
            FReadFile::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FReadFile::Result &Result) {
                OnComplete.ExecuteIfBound(ConvertError(Result.ResultCode));
            })));
        return Ptr->Operation.ToSharedRef();
    }
    default: {
        OnComplete.ExecuteIfBound(
            Errors::NotImplemented(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Unhandled EStorageType value.")));
        return MakeShared<FStorageOperationImpl>();
    }
    }
}

FStorageOperationRef FStorageSystemImpl::WriteStream(
    const UE::Online::FAccountId &LocalUserId,
    const FString &FilePath,
    const FOnWriteChunk &OnChunk,
    const FOnWriteStreamComplete &OnComplete,
    const FOnOperationProgress &OnProgress)
{
    using namespace ::Redpoint::EOS::API::PlayerDataStorage;

    auto Overflow = MakeShared<TArray<uint8>>();
    auto Completed = MakeShared<bool>();

    // We write all files to temporary paths, and then use DuplicateFile/DeleteFile to ensure that writes are atomic on
    // the server.
    auto TempFilePath = FString::Printf(TEXT("__RedpointEOS__/Temp/%s"), *FMD5::HashAnsiString(*FilePath));

    auto Ptr = MakeShared<FStorageOperationPointerHolding>();
    Ptr->Operation = MakeShared<FStorageOperationImpl>(FWriteFile::Execute(
        this->PlatformHandle,
        FWriteFile::Options{GetProductUserId(LocalUserId), TempFilePath, REDPOINT_EOS_STORAGE_CHUNK_SIZE},
        FWriteFile::WriteDelegate::CreateSPLambda(
            this,
            [Ptr, OnChunk, Overflow, Completed](const FWriteFile::WriteResult &Result, TArray<uint8> &OutWrittenData)
                -> FWriteFile::EWriteResult {
                if (StaticCastSharedPtr<FStorageOperationImpl>(Ptr->Operation)->bCancelled)
                {
                    return FWriteFile::EWriteResult::CancelRequest;
                }
                else if (!OnChunk.IsBound())
                {
                    return FWriteFile::EWriteResult::FailRequest;
                }

                auto LengthBytesCast = static_cast<int32>(Result.DataBufferLengthBytes);

                if (Overflow->Num() > 0)
                {
                    auto Take = FMath::Min(Overflow->Num(), LengthBytesCast);
                    OutWrittenData = TArray<uint8>(Overflow->GetData(), Take);
#if REDPOINT_EOS_UE_5_4_OR_LATER
                    Overflow->RemoveAt(0, Take, EAllowShrinking::Yes);
#else
                    Overflow->RemoveAt(0, Take, true);
#endif
                    return (Overflow->Num() == 0 && *Completed) ? FWriteFile::EWriteResult::CompleteRequest
                                                                : FWriteFile::EWriteResult::ContinueWriting;
                }
                else
                {
                    auto Action = OnChunk.Execute(OutWrittenData);
                    if (Action == EWriteStreamAction::Cancel)
                    {
                        return FWriteFile::EWriteResult::CancelRequest;
                    }
                    else if (Action == EWriteStreamAction::Fail)
                    {
                        return FWriteFile::EWriteResult::FailRequest;
                    }
                    if (OutWrittenData.Num() > LengthBytesCast)
                    {
                        // Push to the overflow because we have too much data.
                        *Overflow = TArray<uint8>(
                            OutWrittenData.GetData() + LengthBytesCast,
                            OutWrittenData.Num() - LengthBytesCast);
                        OutWrittenData.SetNum(LengthBytesCast);

                        // We must continue writing because we will be pulling from the overflow.
                        if (Action == EWriteStreamAction::Complete)
                        {
                            *Completed = true;
                        }
                        return FWriteFile::EWriteResult::ContinueWriting;
                    }
                    switch (Action)
                    {
                    case EWriteStreamAction::Continue:
                        return FWriteFile::EWriteResult::ContinueWriting;
                    case EWriteStreamAction::Complete:
                        return FWriteFile::EWriteResult::CompleteRequest;
                    default:
                        return FWriteFile::EWriteResult::FailRequest;
                    }
                }
            }),
        FWriteFile::ProgressDelegate::CreateSPLambda(
            this,
            [OnProgress](const FWriteFile::ProgressResult &Result) {
                OnProgress.ExecuteIfBound(Result.BytesTransferred, Result.TotalFileSizeBytes);
            }),
        FWriteFile::CompletionDelegate::CreateSPLambda(
            this,
            [this, LocalUserId, FilePath, TempFilePath, OnComplete](const FWriteFile::Result &Result) {
                if (Result.ResultCode != EOS_EResult::EOS_Success)
                {
                    OnComplete.ExecuteIfBound(ConvertError(Result.ResultCode));
                    return;
                }

                FDuplicateFile::Execute(
                    this->PlatformHandle,
                    FDuplicateFile::Options{GetProductUserId(LocalUserId), TempFilePath, FilePath},
                    FDuplicateFile::CompletionDelegate::CreateSPLambda(
                        this,
                        [this, LocalUserId, TempFilePath, OnComplete](const FDuplicateFile::Result &DuplicateResult) {
                            if (DuplicateResult.ResultCode != EOS_EResult::EOS_Success)
                            {
                                OnComplete.ExecuteIfBound(ConvertError(DuplicateResult.ResultCode));
                                return;
                            }

                            FDeleteFile::Execute(
                                this->PlatformHandle,
                                FDeleteFile::Options{GetProductUserId(LocalUserId), TempFilePath},
                                FDeleteFile::CompletionDelegate::CreateSPLambda(
                                    this,
                                    [OnComplete](const FDeleteFile::Result &DeleteResult) {
                                        OnComplete.ExecuteIfBound(ConvertError(DeleteResult.ResultCode));
                                    }));
                        }));
            })));
    return Ptr->Operation.ToSharedRef();
}

void FStorageSystemImpl::CancelOperation(const FStorageOperationRef &StorageOperation)
{
    auto Impl = StaticCastSharedRef<FStorageOperationImpl>(StorageOperation);

    UE_LOG(
        LogRedpointEOSStorage,
        Verbose,
        TEXT("Cancellation of operation requested. If the request is cancelled, the original callback will receive a "
             "Canceled result code."));

    Impl->bCancelled = true;

    if (Impl->PlayerDataStorageHandle.IsSet())
    {
        ::Redpoint::EOS::API::PlayerDataStorageFileTransferRequest::FCancelRequest::Execute(
            Impl->PlayerDataStorageHandle.GetValue());
    }
    else if (Impl->TitleStorageHandle.IsSet())
    {
        ::Redpoint::EOS::API::TitleStorageFileTransferRequest::FCancelRequest::Execute(
            Impl->TitleStorageHandle.GetValue());
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()