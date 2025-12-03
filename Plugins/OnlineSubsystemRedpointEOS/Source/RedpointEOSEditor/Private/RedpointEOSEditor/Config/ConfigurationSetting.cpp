// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/ConfigurationSetting.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(827414317, Redpoint::EOS::Editor::Config)
{

FConfigurationSetting::FConfigurationSetting(bool bInValue)
    : bValue(bInValue)
    , IntValue()
    , StrValue()
    , ArrValue()
    , Type(EConfigurationSettingType::Bool)
{
}

FConfigurationSetting::FConfigurationSetting(int32 InValue)
    : bValue()
    , IntValue(InValue)
    , StrValue()
    , ArrValue()
    , Type(EConfigurationSettingType::Int32)
{
}

FConfigurationSetting::FConfigurationSetting(const FString &InValue)
    : bValue()
    , IntValue()
    , StrValue(InValue)
    , ArrValue()
    , Type(EConfigurationSettingType::String)
{
}

FConfigurationSetting::FConfigurationSetting(const TArray<FString> &InValue)
    : bValue()
    , IntValue()
    , StrValue()
    , ArrValue(InValue)
    , Type(EConfigurationSettingType::StringArray)
{
}

EConfigurationSettingType FConfigurationSetting::GetType() const
{
    return this->Type;
}

bool FConfigurationSetting::GetBool() const
{
    check(this->Type == EConfigurationSettingType::Bool);
    return this->bValue;
}

int32 FConfigurationSetting::GetInt32() const
{
    check(this->Type == EConfigurationSettingType::Int32);
    return this->IntValue;
}

const FString &FConfigurationSetting::GetString() const
{
    check(this->Type == EConfigurationSettingType::String);
    return this->StrValue;
}

const TArray<FString> &FConfigurationSetting::GetStringArray() const
{
    check(this->Type == EConfigurationSettingType::StringArray);
    return this->ArrValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()
