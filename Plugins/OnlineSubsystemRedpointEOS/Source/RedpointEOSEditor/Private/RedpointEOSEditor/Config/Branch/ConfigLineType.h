// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(1665466796, Redpoint::EOS::Editor::Config::Branch)
{

enum class EConfigLineType
{
    CommentOrWhitespace,

    Section,

    Set,
    Remove,
    ArrayAddUnique,
    ArrayAdd,
    Clear,
    ArrayOfStructKey,
    POCArrayOfStructKey,
};

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(1665466796, Redpoint::EOS::Editor::Config::Branch, EConfigLineType);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
