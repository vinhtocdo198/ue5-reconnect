// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/ConfigurationWriter.h"
#include "RedpointEOSEditor/Config/ConfigurationWriterPendingChanges.h"
#include "RedpointEOSEditor/Config/Legacy/ConfigurationLoadedFileLegacy.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(353527102, Redpoint::EOS::Editor::Config::Legacy)
{
using namespace ::Redpoint::EOS::Editor::Config;
using namespace ::Redpoint::EOS::Editor::Config::Legacy;

typedef TDelegate<void()> FConfigurationWriterLegacyQueuedDelegate;
typedef TTuple<EConfigurationFileType, FName> FConfigurationWriterLegacyQueuedFileToModify;

class FConfigurationWriterLegacy : public IConfigurationWriter
{
private:
    TArray<FConfigurationWriterLegacyQueuedDelegate> QueuedDelegates;
    TArray<FConfigurationWriterLegacyQueuedFileToModify> FilesQueuedForModify;
    bool bRequireFullReload;
    TMap<FString, TSharedPtr<FConfigurationWriterPendingChanges>> EOSLoadedFiles;

public:
    FConfigurationWriterLegacy();
    UE_NONCOPYABLE(FConfigurationWriterLegacy);
    virtual ~FConfigurationWriterLegacy() override = default;

private:
    FConfigurationLoadedFileLegacy GetConfigFile(const FString &Path);

public:
    virtual bool SetBool(
        const FString &Key,
        bool Value,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;
    virtual bool SetInt32(
        const FString &Key,
        int32 Value,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;
    virtual bool SetString(
        const FString &Key,
        const FString &Value,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;

protected:
    virtual bool SetEnumInternal(
        const FString &Key,
        const FString &EnumClass,
        uint8 Value,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;

public:
    virtual bool ReplaceArray(
        const FString &Key,
        const TArray<FString> &Value,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;

    virtual bool Remove(const FString &Key, const FString &Section, EConfigurationFileType File, FName Platform)
        override;

    [[nodiscard]] virtual EConfigurationWriterFlushResult FlushChanges() override;
};

}

namespace Redpoint::EOS::Editor::Config::Legacy
{
REDPOINT_EOS_FILE_NS_EXPORT(353527102, Redpoint::EOS::Editor::Config::Legacy, FConfigurationWriterLegacy);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
