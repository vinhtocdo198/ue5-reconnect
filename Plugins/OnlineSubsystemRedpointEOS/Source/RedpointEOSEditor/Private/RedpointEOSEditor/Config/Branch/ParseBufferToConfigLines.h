// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/Branch/ConfigLine.h"
#include "RedpointEOSEditor/Config/Branch/ConfigLineType.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(25103830, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config::Branch;

TArray<FConfigLine> ParseBufferToConfigLines(const FString &Contents);

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(25103830, Redpoint::EOS::Editor::Config::Branch, ParseBufferToConfigLines);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
