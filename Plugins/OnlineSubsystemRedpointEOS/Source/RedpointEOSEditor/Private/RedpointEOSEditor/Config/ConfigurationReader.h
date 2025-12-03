// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/ConfigurationConstants.h"
#include "RedpointEOSEditor/Config/ConfigurationFileType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2719159053, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config;

class IConfigurationReader
{
public:
    IConfigurationReader() = default;
    UE_NONCOPYABLE(IConfigurationReader);
    virtual ~IConfigurationReader() = default;

    virtual bool GetBool(
        const FString &Key,
        bool &OutValue,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;
    virtual bool GetInt32(
        const FString &Key,
        int32 &OutValue,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;
    virtual bool GetString(
        const FString &Key,
        FString &OutValue,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;

protected:
    virtual bool GetEnumInternal(
        const FString &Key,
        const FString &EnumClass,
        uint8 &OutValue,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;

public:
    template <typename T>
    bool GetEnum(
        const FString &Key,
        const FString &EnumClass,
        T &OutValue,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None)
    {
        static_assert(sizeof(T) == sizeof(uint8), "must be uint8 enum");
        return GetEnumInternal(Key, EnumClass, (uint8 &)OutValue, Section, File, Platform);
    }

    virtual bool GetArray(
        const FString &Key,
        TArray<FString> &OutValue,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(2719159053, Redpoint::EOS::Editor::Config, IConfigurationReader);
}

REDPOINT_EOS_CODE_GUARD_END()
