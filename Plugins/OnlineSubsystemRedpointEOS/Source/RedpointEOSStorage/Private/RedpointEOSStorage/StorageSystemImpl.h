// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSStorage/StorageSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2118794800, Redpoint::EOS::Storage)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Storage;

class REDPOINTEOSSTORAGE_API FStorageSystemImpl : public IStorageSystem, public TSharedFromThis<FStorageSystemImpl>
{
private:
    FPlatformHandle PlatformHandle;

public:
    FStorageSystemImpl(const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FStorageSystemImpl);
    virtual ~FStorageSystemImpl() override = default;

    virtual void List(
        const TOptional<UE::Online::FAccountId> &LocalUserId,
        EStorageType StorageType,
        const TArray<FString> &TitleStorageTags,
        const FOnListComplete &OnComplete) override;
    virtual FStorageOperationRef Read(
        const TOptional<UE::Online::FAccountId> &LocalUserId,
        EStorageType StorageType,
        const FString &FilePath,
        const FOnReadComplete &OnComplete,
        const FOnOperationProgress &OnProgress) override;
    virtual FStorageOperationRef Write(
        const UE::Online::FAccountId &LocalUserId,
        const FString &FilePath,
        const TArray<uint8> &FileData,
        const FOnWriteComplete &OnComplete,
        const FOnOperationProgress &OnProgress) override;
    virtual void Delete(
        const UE::Online::FAccountId &LocalUserId,
        const FString &FilePath,
        const FOnDeleteComplete &OnComplete) override;

    virtual FStorageOperationRef ReadStream(
        const TOptional<UE::Online::FAccountId> &LocalUserId,
        EStorageType StorageType,
        const FString &FilePath,
        const FOnReadChunk &OnChunk,
        const FOnReadStreamComplete &OnComplete,
        const FOnOperationProgress &OnProgress) override;
    virtual FStorageOperationRef WriteStream(
        const UE::Online::FAccountId &LocalUserId,
        const FString &FilePath,
        const FOnWriteChunk &OnChunk,
        const FOnWriteStreamComplete &OnComplete,
        const FOnOperationProgress &OnProgress) override;

    virtual void CancelOperation(const FStorageOperationRef &StorageOperation) override;
};

}

namespace Redpoint::EOS::Storage
{
REDPOINT_EOS_FILE_NS_EXPORT(2118794800, Redpoint::EOS::Storage, FStorageSystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()