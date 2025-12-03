// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/TestMapName.h"

#include "HAL/PlatformProcess.h"
#include "Misc/CommandLine.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2585612036, Redpoint::EOS::Editor::Tests)
{

FString FTestMapName::GetMapName(int32 Id)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("unattended")))
    {
        return FString::Printf(TEXT("AutomatedTestMap%d"), Id);
    }
    else
    {
        // When test processes run in parallel, we have to make sure multiple processes don't all try to use/save the
        // same map.
        return FString::Printf(TEXT("AutomatedTestMap%d_%d"), Id, FPlatformProcess::GetCurrentProcessId());
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()
