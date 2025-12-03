// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if PLATFORM_WINDOWS || PLATFORM_MAC
#include "Containers/Ticker.h"
#include "Containers/UnrealString.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_WINDOWS || PLATFORM_MAC

namespace REDPOINT_EOS_FILE_NS_ID(314590786, Redpoint::EOS::Editor)
{

class FDevAuthToolLauncher
{
private:
    static FDevAuthToolLauncher Instance;

public:
    static FDevAuthToolLauncher &Get();

private:
    bool bNeedsToCheckIfDevToolRunning;
    bool bIsDevToolRunning;
    FTSTicker::FDelegateHandle ResetCheckHandle;

    FDevAuthToolLauncher();
    UE_NONCOPYABLE(FDevAuthToolLauncher);
    ~FDevAuthToolLauncher() = default;

    static bool ClearKnowledgeThatDevAuthToolIsRunning(float);
    void ScheduleClearKnowledgeThatDevAuthToolIsRunning();

    FString GetShortestPathFromRelativePath(const FString &relativePath);
    FString GetPathToToolsFolder();

    bool ExtractDeveloperAuthenticationTool(const FString &Path, bool bInteractive);

public:
    bool IsDevAuthToolRunning();

    bool Launch(bool bInteractive, bool bForceReinstall = false);
};

}

namespace Redpoint::EOS::Editor
{
REDPOINT_EOS_FILE_NS_EXPORT(314590786, Redpoint::EOS::Editor, FDevAuthToolLauncher);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()