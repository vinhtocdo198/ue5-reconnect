// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/Branch/ConfigLine.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(4250419609, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config::Branch;

/**
 * Represents a section in a file. There may be multiple sections with the same name. The first entry of LinesInSection
 * is always the original [Section] line.
 */
struct FConfigIndexSection
{
    FString SectionName;
    TArray<FConfigLine> LinesInSection;
    TSet<FString> ConfigKeysPresent;
};

struct FConfigIndex
{
    TArray<FConfigLine> PreSectionLines;
    TArray<FConfigIndexSection> Sections;
    TMap<FString, int32> SectionIndex;
};

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(4250419609, Redpoint::EOS::Editor::Config::Branch, FConfigIndexSection);
REDPOINT_EOS_FILE_NS_EXPORT(4250419609, Redpoint::EOS::Editor::Config::Branch, FConfigIndex);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
