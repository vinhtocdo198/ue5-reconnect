// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSStorage/FileEntry.h"
#include "RedpointEOSStorage/ReadStreamAction.h"
#include "RedpointEOSStorage/StorageOperation.h"
#include "RedpointEOSStorage/StorageType.h"
#include "RedpointEOSStorage/WriteStreamAction.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(290799007, Redpoint::EOS::Storage)
{
using namespace ::Redpoint::EOS::Storage;
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSSTORAGE_API IStorageSystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<IStorageSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IStorageSystem() = default;
    UE_NONCOPYABLE(IStorageSystem);
    virtual ~IStorageSystem() override = default;

    typedef void *FOperationHandle;

    typedef TDelegate<void(const FError &ErrorCode, const TArray<FFileEntry> &FileList)> FOnListComplete;
    typedef TDelegate<void(uint32 BytesTransferred, uint32 TotalFileSizeBytes)> FOnOperationProgress;
    typedef TDelegate<void(const FError &ErrorCode, const TArray<uint8> &FileData)> FOnReadComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnWriteComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnDeleteComplete;

    /**
     * List all of the files.
     *
     * @param LocalUserId		The local user that wants to list files. This value can be unset for Title Storage, in
     *							which case files are listed without authentication.
     * @param StorageType		Indicates whether to list files from Player Data Storage or Title Storage.
     * @param TitleStorageTags  If querying Title Storage, these are the tags to query for. If empty, defaults to a
     *							single entry of "Default".
     * @param OnComplete		Called when the operation completes, with the full list of file paths on success.
     */
    virtual void List(
        const TOptional<UE::Online::FAccountId> &LocalUserId,
        EStorageType StorageType,
        const TArray<FString> &TitleStorageTags,
        const FOnListComplete &OnComplete) = 0;

    /**
     * Read the entire contents of a single file.
     *
     * @param LocalUserId	The local user that wants to read a file. This value can be unset for Title Storage, in
     *						which case the file is read without authentication.
     * @param StorageType	Indicates whether to read the file from Player Data Storage or Title Storage.
     * @param FilePath		The path of the file to read.
     * @param OnComplete	Called when the operation completes, with the entire file contents on success.
     * @param OnProgress	An optional callback that indicates the progress of the download.
     */
    virtual FStorageOperationRef Read(
        const TOptional<UE::Online::FAccountId> &LocalUserId,
        EStorageType StorageType,
        const FString &FilePath,
        const FOnReadComplete &OnComplete,
        const FOnOperationProgress &OnProgress = FOnOperationProgress()) = 0;

    /**
     * Write a single file to Player Data Storage, passing the entire contents at once.
     *
     * @param LocalUserId	The local user that wants to write the file.
     * @param FilePath		The path of the file to write.
     * @param FileData		The entire file data to set as the contents.
     * @param OnComplete	Called when the operation completes.
     * @param OnProgress	An optional callback that indicates the progress of the download.
     */
    virtual FStorageOperationRef Write(
        const UE::Online::FAccountId &LocalUserId,
        const FString &FilePath,
        const TArray<uint8> &FileData,
        const FOnWriteComplete &OnComplete,
        const FOnOperationProgress &OnProgress = FOnOperationProgress()) = 0;

    /**
     * Delete a single file from Player Data Storage.
     *
     * @param LocalUserId	The local user that wants to write the file.
     * @param FilePath		The path of the file to write.
     * @param OnComplete	Called when the operation completes.
     */
    virtual void Delete(
        const UE::Online::FAccountId &LocalUserId,
        const FString &FilePath,
        const FOnDeleteComplete &OnComplete) = 0;

    typedef TDelegate<EReadStreamAction(const TArray<uint8> &ChunkData, uint32 TotalFileSizeBytes, bool bIsLastChunk)>
        FOnReadChunk;
    typedef TDelegate<EWriteStreamAction(TArray<uint8> &ChunkData)> FOnWriteChunk;
    typedef TDelegate<void(const FError &ErrorCode)> FOnReadStreamComplete;
    typedef TDelegate<void(const FError &ErrorCode)> FOnWriteStreamComplete;

    /**
     * Read a single file, streaming each chunk of data as it's available. You should use this function if the file is
     * potentially too large to store in memory at once.
     *
     * @param LocalUserId	The local user that wants to read a file. This value can be unset for Title Storage, in
     *						which case the file is read without authentication.
     * @param StorageType	Indicates whether to read the file from Player Data Storage or Title Storage.
     * @param FilePath		The path of the file to read.
     * @param OnChunk		Called when the next read chunk of the file is available to process.
     * @param OnComplete	Called when the read operation fully completes.
     * @param OnProgress	An optional callback that indicates the progress of the download.
     */
    virtual FStorageOperationRef ReadStream(
        const TOptional<UE::Online::FAccountId> &LocalUserId,
        EStorageType StorageType,
        const FString &FilePath,
        const FOnReadChunk &OnChunk,
        const FOnReadStreamComplete &OnComplete,
        const FOnOperationProgress &OnProgress = FOnOperationProgress()) = 0;

    /**
     * Write a single file, providing each chunk of data on demand as the EOS SDK requires it. You should use this
     * function if you're writing a large file that you don't already have entirely in memory.
     *
     * @param LocalUserId	The local user that wants to write the file.
     * @param FilePath		The path of the file to write.
     * @param OnChunk		Called when the next chunk of the file needs to be written. You're expected to set the data
     *						into the ChunkData parameter.
     * @param OnComplete	Called when the write operation fully completes.
     * @param OnProgress	An optional callback that indicates the progress of the download.
     */
    virtual FStorageOperationRef WriteStream(
        const UE::Online::FAccountId &LocalUserId,
        const FString &FilePath,
        const FOnWriteChunk &OnChunk,
        const FOnWriteStreamComplete &OnComplete,
        const FOnOperationProgress &OnProgress = FOnOperationProgress()) = 0;

    /**
     * Cancel an existing read or write operation. If the cancellation is successful, the completion delegate that was
     * originally passed to the operation will receive an error code of "Canceled".
     */
    virtual void CancelOperation(const FStorageOperationRef &StorageOperation) = 0;
};

}

namespace Redpoint::EOS::Storage
{
REDPOINT_EOS_FILE_NS_EXPORT(290799007, Redpoint::EOS::Storage, IStorageSystem)
}

REDPOINT_EOS_CODE_GUARD_END()