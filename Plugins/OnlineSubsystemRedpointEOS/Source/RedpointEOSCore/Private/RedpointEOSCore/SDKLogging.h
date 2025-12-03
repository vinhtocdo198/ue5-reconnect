// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1736942110, Redpoint::EOS::Core)
{

class FSDKLogging
{
public:
#if WITH_EDITOR
    typedef TDelegate<void(int32_t, const FString &, const FString &)> FOnLogForwardedForEditor;
    static FOnLogForwardedForEditor EditorLogHandler;
#endif

    static void Log(const EOS_LogMessage *Message);
};

}

namespace Redpoint::EOS::Core
{
REDPOINT_EOS_FILE_NS_EXPORT(1736942110, Redpoint::EOS::Core, FSDKLogging)
}

REDPOINT_EOS_CODE_GUARD_END()