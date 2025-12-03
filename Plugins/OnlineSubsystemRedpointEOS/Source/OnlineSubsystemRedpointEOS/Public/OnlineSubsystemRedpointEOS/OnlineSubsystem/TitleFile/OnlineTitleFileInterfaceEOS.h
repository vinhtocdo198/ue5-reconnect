// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineTitleFileInterface.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Id/UserIdMap.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(496971326, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineTitleFileInterfaceEOS : public IOnlineTitleFile,
                                                                    public TSharedFromThis<FOnlineTitleFileInterfaceEOS>
{
private:
    FPlatformHandle PlatformHandle;
    TArray<FCloudFileHeader> ListCache;
    TMap<FString, TArray<uint8>> DownloadCache;

    TSharedRef<const FUniqueNetIdEOS> GetLocalUserId() const;

public:
    FOnlineTitleFileInterfaceEOS(const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FOnlineTitleFileInterfaceEOS);
    virtual ~FOnlineTitleFileInterfaceEOS() override = default;

    virtual bool GetFileContents(const FString &FileName, TArray<uint8> &FileContents) override;
    virtual bool ClearFiles() override;
    virtual bool ClearFile(const FString &FileName) override;
    virtual void DeleteCachedFiles(bool bSkipEnumerated) override;
    virtual bool EnumerateFiles(const FPagedQuery &Page) override;
    virtual void GetFileList(TArray<FCloudFileHeader> &UserFiles) override;
    virtual bool ReadFile(const FString &FileName) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile
{
REDPOINT_EOS_FILE_NS_EXPORT(
    496971326,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile,
    FOnlineTitleFileInterfaceEOS)
}

REDPOINT_EOS_CODE_GUARD_END()