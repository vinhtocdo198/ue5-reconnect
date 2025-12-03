// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/Branch/ConfigIndex.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(4225613344, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config::Branch;

/**
 * Generates an index of the config lines. All FConfigLine entries are guaranteed to be in order that they were
 * originally present in the file.
 */
FConfigIndex IndexConfigLines(const TArray<FConfigLine> &ConfigLines);

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(4225613344, Redpoint::EOS::Editor::Config::Branch, IndexConfigLines);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
