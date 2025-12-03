// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(938180516, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests;

class FJoinSessionChainedTask : public TestUtilities::ChainedTask::FChainedTask,
                                public TSharedFromThis<FJoinSessionChainedTask>
{
private:
    int32 JoiningInstanceId;
    FName SessionName;
    TSharedRef<FOnlineSessionSearchResult> FoundSearchResult;
    FDelegateHandle OnJoinSessionCompleteHandle;

public:
    FJoinSessionChainedTask(
        int32 InJoiningInstanceId,
        const FName &InSessionName,
        const TSharedRef<FOnlineSessionSearchResult> &InFoundSearchResult);
    UE_NONCOPYABLE(FJoinSessionChainedTask);
    virtual ~FJoinSessionChainedTask() override = default;

    virtual void ExecuteAsync(const TestUtilities::FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnJoinSessionComplete(
        FName CallbackSessionName,
        EOnJoinSessionCompleteResult::Type CallbackJoinResult,
        TestUtilities::FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(938180516, Redpoint::EOS::Tests::Session, FJoinSessionChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()