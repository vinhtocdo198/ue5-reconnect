// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/ConfigurationWriter.h"
#include "RedpointEOSEditor/Config/ConfigurationWriterPendingChanges.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2436931538, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config;

class FConfigurationWriterBranch : public IConfigurationWriter
{
private:
    TMap<FString, FConfigurationWriterPendingChanges> PendingChanges;

public:
    FConfigurationWriterBranch();
    UE_NONCOPYABLE(FConfigurationWriterBranch);
    virtual ~FConfigurationWriterBranch() override = default;

private:
    FConfigurationWriterPendingChanges &GetConfigFile(EConfigurationFileType File, FName Platform);

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

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(2436931538, Redpoint::EOS::Editor::Config::Branch, FConfigurationWriterBranch);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
