// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/DelegateCombinations.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2425966901, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks)
{

typedef TDelegate<void()> FVoiceTaskComplete;

class FVoiceTask
{
public:
    FVoiceTask() = default;
    UE_NONCOPYABLE(FVoiceTask);
    virtual ~FVoiceTask() = default;

    virtual FString GetLogName() = 0;
    virtual void Run(FVoiceTaskComplete OnComplete) = 0;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks
{
REDPOINT_EOS_FILE_NS_EXPORT(2425966901, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks, FVoiceTaskComplete)
REDPOINT_EOS_FILE_NS_EXPORT(2425966901, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks, FVoiceTask)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()