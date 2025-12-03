// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/Branch/ConfigLineType.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(1507507019, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config::Branch;

struct FConfigLine
{
    FString LogicalLine;
    FString PhysicalLine;
    FString PhysicalLineTrailingComment;
    bool bIsWindowsNewlineTerminated;
    EConfigLineType Type;
    FString SectionName;
    FString SettingName;
    FString SettingValue;

    FConfigLine(
        const FString &InLogicalLine,
        const FString &InPhysicalLine,
        const FString &InPhysicalLineTrailingComment,
        bool bInIsWindowsNewlineTerminated)
        : LogicalLine(InLogicalLine)
        , PhysicalLine(InPhysicalLine)
        , PhysicalLineTrailingComment(InPhysicalLineTrailingComment)
        , bIsWindowsNewlineTerminated(bInIsWindowsNewlineTerminated)
        , Type(EConfigLineType::CommentOrWhitespace)
        , SectionName()
        , SettingName()
        , SettingValue()
    {
    }
};

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(1507507019, Redpoint::EOS::Editor::Config::Branch, FConfigLine);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
