// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/ConfigurationSettingType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(827414317, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config;

class FConfigurationSetting
{
private:
    bool bValue;
    int32 IntValue;
    FString StrValue;
    TArray<FString> ArrValue;
    EConfigurationSettingType Type;

public:
    FConfigurationSetting(bool bInValue);
    FConfigurationSetting(int32 InValue);
    FConfigurationSetting(const FString &InValue);
    FConfigurationSetting(const TArray<FString> &InValue);

    EConfigurationSettingType GetType() const;
    bool GetBool() const;
    int32 GetInt32() const;
    const FString &GetString() const;
    const TArray<FString> &GetStringArray() const;
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(827414317, Redpoint::EOS::Editor::Config, FConfigurationSetting)
}

REDPOINT_EOS_CODE_GUARD_END()
