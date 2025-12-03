// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Legacy/ConfigurationReaderLegacy.h"

#if !REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/ConfigCacheIni.h"
#include "RedpointEOSConfig/EngineConfigHelpers.h"
#include "RedpointEOSEditor/Config/ConfigurationEnumLookup.h"
#include "RedpointEOSEditor/Config/ConfigurationFilePathHelpers.h"
#include "RedpointEOSEditorModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2546342926, Redpoint::EOS::Editor::Config::Legacy)
{
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Editor::Config;

bool FConfigurationReaderLegacy::GetBool(
    const FString &Key,
    bool &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigFile *F = FEngineConfigHelpers::FindByName(GetFilePath(File, Platform));
    if (!F)
    {
        return false;
    }
    return F->GetBool(*Section, *Key, OutValue);
}

bool FConfigurationReaderLegacy::GetInt32(
    const FString &Key,
    int32 &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigFile *F = FEngineConfigHelpers::FindByName(GetFilePath(File, Platform));
    if (!F)
    {
        return false;
    }
    return F->GetInt(*Section, *Key, OutValue);
}

bool FConfigurationReaderLegacy::GetString(
    const FString &Key,
    FString &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigFile *F = FEngineConfigHelpers::FindByName(GetFilePath(File, Platform));
    if (!F)
    {
        return false;
    }
    return F->GetString(*Section, *Key, OutValue);
}

bool FConfigurationReaderLegacy::GetEnumInternal(
    const FString &Key,
    const FString &EnumClass,
    uint8 &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigFile *F = FEngineConfigHelpers::FindByName(GetFilePath(File, Platform));
    if (!F)
    {
        return false;
    }
    FString EnumValue;
    if (!F->GetString(*Section, *Key, EnumValue))
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

bool FConfigurationReaderLegacy::GetArray(
    const FString &Key,
    TArray<FString> &OutValue,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigFile *F = FEngineConfigHelpers::FindByName(GetFilePath(File, Platform));
    if (!F)
    {
        return false;
    }
    // Try loading with the + prefix, which is now the correct way to read things
    // from a raw (not virtualized) config file.
    if (F->GetArray(*Section, *FString::Printf(TEXT("+%s"), *Key), OutValue) == 0)
    {
        // We didn't get anything with the proper way. Try legacy configuration values
        // so that we import them correctly.
        return F->GetArray(*Section, *Key, OutValue) > 0;
    }
    else
    {
        return true;
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
