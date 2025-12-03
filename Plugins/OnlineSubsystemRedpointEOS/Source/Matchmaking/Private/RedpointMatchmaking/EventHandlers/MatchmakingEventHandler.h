// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "../MatchmakingEngineTask.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class IMatchmakingEventHandler : public TSharedFromThis<IMatchmakingEventHandler>
{
protected:
    /**
     * Gets the log prefix to use for logging.
     */
    FString GetLogPrefix(const FMatchmakingEngineTask &Task, const FName EventHandlerName) const
    {
        return FString::Printf(TEXT("Task %s: %s: "), *Task.TaskId, *EventHandlerName.ToString());
    }

public:
    virtual ~IMatchmakingEventHandler() = default;
    UE_NONCOPYABLE(IMatchmakingEventHandler);
    IMatchmakingEventHandler() = default;

    virtual void RegisterEvents(class FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) = 0;
    virtual void UnregisterEvents(class FMatchmakingEngineImpl &Engine, FMatchmakingEngineTask &Task) = 0;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()