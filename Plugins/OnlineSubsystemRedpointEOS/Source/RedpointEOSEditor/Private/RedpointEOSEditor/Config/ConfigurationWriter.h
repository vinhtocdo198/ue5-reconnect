// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/ConfigurationConstants.h"
#include "RedpointEOSEditor/Config/ConfigurationFileType.h"
#include "RedpointEOSEditor/Config/ConfigurationWriterFlushResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2966887204, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config;

class IConfigurationWriter
{
public:
    IConfigurationWriter() = default;
    UE_NONCOPYABLE(IConfigurationWriter);
    virtual ~IConfigurationWriter() = default;

    virtual bool SetBool(
        const FString &Key,
        bool Value,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;
    virtual bool SetInt32(
        const FString &Key,
        int32 Value,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;
    virtual bool SetString(
        const FString &Key,
        const FString &Value,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;

protected:
    virtual bool SetEnumInternal(
        const FString &Key,
        const FString &EnumClass,
        uint8 Value,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;

public:
    template <typename T>
    bool SetEnum(
        const FString &Key,
        const FString &EnumClass,
        T Value,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None)
    {
        static_assert(sizeof(T) == sizeof(uint8), "must be uint8 enum");
        return SetEnumInternal(Key, EnumClass, (uint8)Value, Section, File, Platform);
    }

    virtual bool ReplaceArray(
        const FString &Key,
        const TArray<FString> &Value,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;

    virtual bool Remove(
        const FString &Key,
        const FString &Section = DefaultEOSSection,
        EConfigurationFileType File = EConfigurationFileType::Engine,
        FName Platform = NAME_None) = 0;

    [[nodiscard]] virtual EConfigurationWriterFlushResult FlushChanges() = 0;
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(2966887204, Redpoint::EOS::Editor::Config, IConfigurationWriter);
}

REDPOINT_EOS_CODE_GUARD_END()
