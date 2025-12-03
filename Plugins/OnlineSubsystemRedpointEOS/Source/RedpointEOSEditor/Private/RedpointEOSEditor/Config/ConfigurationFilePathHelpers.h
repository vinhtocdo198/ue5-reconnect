// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/ConfigurationFileType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3436934203, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config;

extern TMap<FString, FString> FilePathCache;

FString GetFilePath(EConfigurationFileType File, FName Platform);

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(3436934203, Redpoint::EOS::Editor::Config, FilePathCache)
REDPOINT_EOS_FILE_NS_EXPORT(3436934203, Redpoint::EOS::Editor::Config, GetFilePath)
}

REDPOINT_EOS_CODE_GUARD_END()
