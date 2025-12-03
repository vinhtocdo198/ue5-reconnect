// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Class.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(474878901, Redpoint::EOS::Editor::Config)
{

const UEnum *LookupEnum(const FString &EnumClass);

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(474878901, Redpoint::EOS::Editor::Config, LookupEnum)
}

REDPOINT_EOS_CODE_GUARD_END()