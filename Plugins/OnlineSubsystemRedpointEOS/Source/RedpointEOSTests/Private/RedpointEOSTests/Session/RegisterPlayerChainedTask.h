// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "OnlineSessionSettings.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2225559797, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests;

class FRegisterPlayerChainedTask : public TestUtilities::ChainedTask::FChainedTask,
                                   public TSharedFromThis<FRegisterPlayerChainedTask>
{
private:
    int32 HostingInstanceId;
    int32 TargetInstanceId;
    FName SessionName;
    FDelegateHandle OnRegisterPlayersCompleteHandle;

public:
    FRegisterPlayerChainedTask(int32 InHostingInstanceId, int32 InTargetInstanceId, const FName &InSessionName);
    UE_NONCOPYABLE(FRegisterPlayerChainedTask);
    virtual ~FRegisterPlayerChainedTask() override = default;

    virtual void ExecuteAsync(const TestUtilities::FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnRegisterPlayersComplete(
        FName CallbackSessionName,
        const TArray<FUniqueNetIdRef> &CallbackRegisteredPlayerIds,
        bool bCallbackWasSuccessful,
        TestUtilities::FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(2225559797, Redpoint::EOS::Tests::Session, FRegisterPlayerChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()