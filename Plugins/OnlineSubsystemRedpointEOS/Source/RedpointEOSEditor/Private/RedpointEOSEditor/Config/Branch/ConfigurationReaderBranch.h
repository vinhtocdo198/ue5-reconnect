// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/ConfigCacheIni.h"
#include "RedpointEOSEditor/Config/ConfigurationReader.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(1739253288, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config;

class FConfigurationReaderBranch : public IConfigurationReader
{
public:
    FConfigurationReaderBranch() = default;
    UE_NONCOPYABLE(FConfigurationReaderBranch);
    virtual ~FConfigurationReaderBranch() override = default;

    virtual bool GetBool(
        const FString &Key,
        bool &OutValue,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;
    virtual bool GetInt32(
        const FString &Key,
        int32 &OutValue,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;
    virtual bool GetString(
        const FString &Key,
        FString &OutValue,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;

protected:
    virtual bool GetEnumInternal(
        const FString &Key,
        const FString &EnumClass,
        uint8 &OutValue,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;

public:
    virtual bool GetArray(
        const FString &Key,
        TArray<FString> &OutValue,
        const FString &Section,
        EConfigurationFileType File,
        FName Platform) override;
};

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(1739253288, Redpoint::EOS::Editor::Config::Branch, FConfigurationReaderBranch);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
