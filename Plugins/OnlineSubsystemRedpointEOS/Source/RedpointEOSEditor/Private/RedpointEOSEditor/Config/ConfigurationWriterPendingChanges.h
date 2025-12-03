// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/ConfigurationFileType.h"
#include "RedpointEOSEditor/Config/ConfigurationSectionKey.h"
#include "RedpointEOSEditor/Config/ConfigurationSetting.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2242189668, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config;

struct FConfigurationWriterPendingChanges
{
public:
#if REDPOINT_EOS_UE_5_5_OR_LATER
    FConfigurationWriterPendingChanges(const TTuple<FString, EConfigurationFileType, FName> &InitFromMap);
#else
    FConfigurationWriterPendingChanges(const FString &InPath);
#endif
    UE_NONCOPYABLE(FConfigurationWriterPendingChanges);
    FString Path;
#if REDPOINT_EOS_UE_5_5_OR_LATER
    EConfigurationFileType File;
    FName Platform;
#endif
    TSet<FConfigurationSectionKey> Removals;
    TMap<FConfigurationSectionKey, FConfigurationSetting> Upserts;
    TMap<FConfigurationSectionKey, FConfigurationSetting> Replacements;
    bool bIsModified;
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(2242189668, Redpoint::EOS::Editor::Config, FConfigurationWriterPendingChanges)
}

REDPOINT_EOS_CODE_GUARD_END()
