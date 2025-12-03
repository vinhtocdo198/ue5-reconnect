// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/ConfigurationSettingType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(269708907, Redpoint::EOS::Editor::Config)
{

struct FConfigurationSectionKey
{
    FConfigurationSectionKey(const FString &InSection, const FString &InKey);
    FString Section;
    FString Key;

    bool operator==(const FConfigurationSectionKey &Other) const;
    friend uint32 GetTypeHash(const FConfigurationSectionKey &Val);
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(269708907, Redpoint::EOS::Editor::Config, FConfigurationSectionKey)
}

REDPOINT_EOS_CODE_GUARD_END()
