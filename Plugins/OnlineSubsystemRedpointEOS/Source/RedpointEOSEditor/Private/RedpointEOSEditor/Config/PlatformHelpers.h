// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1786651394, Redpoint::EOS::Editor::Config)
{

TArray<FName> GetAllPlatformNames();
TArray<FName> GetConfidentialPlatformNames();

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(1786651394, Redpoint::EOS::Editor::Config, GetAllPlatformNames)
REDPOINT_EOS_FILE_NS_EXPORT(1786651394, Redpoint::EOS::Editor::Config, GetConfidentialPlatformNames)
}

REDPOINT_EOS_CODE_GUARD_END()