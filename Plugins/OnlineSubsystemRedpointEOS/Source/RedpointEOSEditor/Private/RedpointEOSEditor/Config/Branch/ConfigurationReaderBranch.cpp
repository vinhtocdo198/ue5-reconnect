// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Branch/ConfigurationReaderBranch.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSConfig/EngineConfigHelpers.h"
#include "RedpointEOSEditor/Config/Branch/ConfigurationStreamAccessor.h"
#include "RedpointEOSEditor/Config/ConfigurationEnumLookup.h"
#include "RedpointEOSEditor/Config/ConfigurationFilePathHelpers.h"
#include "RedpointEOSEditorModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(1739253288, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Editor::Config;
using namespace ::Redpoint::EOS::Editor::Config::Branch;

bool FConfigurationReaderBranch::GetBool(
    const FString &Key,
    bool &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FString StringValue;
    if (!this->GetString(Key, StringValue, Section, File, Platform))
    {
        return false;
    }
    OutValue = FCString::ToBool(*StringValue);
    return true;
}

bool FConfigurationReaderBranch::GetInt32(
    const FString &Key,
    int32 &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FString StringValue;
    if (!this->GetString(Key, StringValue, Section, File, Platform))
    {
        return false;
    }
    OutValue = FCString::Atoi(*StringValue);
    return true;
}

bool FConfigurationReaderBranch::GetString(
    const FString &Key,
    FString &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FString FilePath;
    FConfigBranch *Branch = FConfigurationStreamAccessor::GetConfigBranchUnchecked(File, Platform, FilePath);
    if (Branch == nullptr)
    {
        return false;
    }
    const FConfigCommandStream *Stream = FConfigurationStreamAccessor::FindStaticLayer(Branch, FilePath);
    if (Stream == nullptr)
    {
        return false;
    }
    const FConfigCommandStreamSection *StreamSection = Stream->Find(Section);
    if (StreamSection == nullptr)
    {
        return false;
    }
    const FConfigValue *StreamSectionValue = StreamSection->Find(FName(*Key));
    if (StreamSectionValue == nullptr)
    {
        return false;
    }
    OutValue = StreamSectionValue->GetValue();
    return true;
}

bool FConfigurationReaderBranch::GetEnumInternal(
    const FString &Key,
    const FString &EnumClass,
    uint8 &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FString EnumValue;
    if (!this->GetString(Key, EnumValue, Section, File, Platform))
    {
        return false;
    }

    const UEnum *Enum = LookupEnum(EnumClass);
    if (Enum == nullptr)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Error,
            TEXT("Unable to retrieve UObject metadata for enum %s, so the current value of %s can't be read."),
            *EnumClass,
            *Key);
        return false;
    }
    OutValue = (uint8)Enum->GetValueByName(FName(*EnumValue), EGetByNameFlags::ErrorIfNotFound);
    return true;
}

bool FConfigurationReaderBranch::GetArray(
    const FString &Key,
    TArray<FString> &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FString FilePath;
    FConfigBranch *Branch = FConfigurationStreamAccessor::GetConfigBranchUnchecked(File, Platform, FilePath);
    if (Branch == nullptr)
    {
        return false;
    }
    const FConfigCommandStream *Stream = FConfigurationStreamAccessor::FindStaticLayer(Branch, FilePath);
    if (Stream == nullptr)
    {
        return false;
    }
    const FConfigCommandStreamSection *StreamSection = Stream->Find(Section);
    if (StreamSection == nullptr)
    {
        return false;
    }
    TArray<FConfigValue> FoundValues;
    StreamSection->MultiFind(FName(*Key), FoundValues, true);
    for (const auto &FoundValue : FoundValues)
    {
        if (FoundValue.ValueType == FConfigValue::EValueType::Clear)
        {
            OutValue.Empty();
        }
        else if (
            FoundValue.ValueType == FConfigValue::EValueType::ArrayAdd ||
            FoundValue.ValueType == FConfigValue::EValueType::Set)
        {
            OutValue.Add(FoundValue.GetValue());
        }
        else if (FoundValue.ValueType == FConfigValue::EValueType::ArrayAddUnique)
        {
            OutValue.AddUnique(FoundValue.GetValue());
        }
        else if (FoundValue.ValueType == FConfigValue::EValueType::Remove)
        {
            OutValue.Remove(FoundValue.GetValue());
        }
        else
        {
            checkf(false, TEXT("Unsupported ValueType for array config value!"));
        }
    }
    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
