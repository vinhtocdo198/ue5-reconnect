// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2585612036, Redpoint::EOS::Editor::Tests)
{

class FTestMapName
{
public:
    static FString GetMapName(int32 Id);
};

}

namespace Redpoint::EOS::Editor::Tests
{
REDPOINT_EOS_FILE_NS_EXPORT(2585612036, Redpoint::EOS::Editor::Tests, FTestMapName)
}

REDPOINT_EOS_CODE_GUARD_END()