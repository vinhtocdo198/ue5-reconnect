// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlineUserCloudInterface.h"
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#include "RedpointEOSStorage/StorageOperation.h"
#include "RedpointEOSStorage/StorageSystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(513699043, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::UserCloud)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Storage;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineUserCloudInterfaceEOS : public IOnlineUserCloud,
                                                                    public TSharedFromThis<FOnlineUserCloudInterfaceEOS>
{
private:
    FPlatformHandle PlatformHandle;
    TUserIdMap<TArray<FCloudFileHeader>> ListCache;
    TUserIdMap<TMap<FString, TArray<uint8>>> DownloadCache;
    TUserIdMap<TMap<FString, FStorageOperationRef>> WriteOperations;

public:
    FOnlineUserCloudInterfaceEOS(const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FOnlineUserCloudInterfaceEOS);
    virtual ~FOnlineUserCloudInterfaceEOS() override = default;

    virtual bool GetFileContents(const FUniqueNetId &UserId, const FString &FileName, TArray<uint8> &FileContents)
        override;
    virtual bool ClearFiles(const FUniqueNetId &UserId) override;
    virtual bool ClearFile(const FUniqueNetId &UserId, const FString &FileName) override;
    virtual void EnumerateUserFiles(const FUniqueNetId &UserId) override;
    virtual void GetUserFileList(const FUniqueNetId &UserId, TArray<FCloudFileHeader> &UserFiles) override;
    virtual bool ReadUserFile(const FUniqueNetId &UserId, const FString &FileName) override;
    virtual bool WriteUserFile(
        const FUniqueNetId &UserId,
        const FString &FileName,
        TArray<uint8> &FileContents,
        bool bCompressBeforeUpload) override;
    virtual void CancelWriteUserFile(const FUniqueNetId &UserId, const FString &FileName) override;
    virtual bool DeleteUserFile(
        const FUniqueNetId &UserId,
        const FString &FileName,
        bool bShouldCloudDelete,
        bool bShouldLocallyDelete) override;
    virtual bool RequestUsageInfo(const FUniqueNetId &UserId) override;
    virtual void DumpCloudState(const FUniqueNetId &UserId) override;
    virtual void DumpCloudFileState(const FUniqueNetId &UserId, const FString &FileName) override;

    void ReadUserFileToCache(
        const UE::Online::FAccountId &AccountId,
        const FString &FileName,
        const IStorageSystem::FOnReadComplete &OnComplete);
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::UserCloud
{
REDPOINT_EOS_FILE_NS_EXPORT(
    513699043,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::UserCloud,
    FOnlineUserCloudInterfaceEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()