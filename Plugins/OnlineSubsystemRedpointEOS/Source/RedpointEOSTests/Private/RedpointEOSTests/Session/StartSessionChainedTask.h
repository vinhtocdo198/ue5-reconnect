// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "OnlineSessionSettings.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3102801617, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests;

class FStartSessionChainedTask : public TestUtilities::ChainedTask::FChainedTask,
                                 public TSharedFromThis<FStartSessionChainedTask>
{
private:
    int32 HostingInstanceId;
    FName SessionName;
    FDelegateHandle OnStartSessionCompleteHandle;

public:
    FStartSessionChainedTask(int32 InHostingInstanceId, const FName &InSessionName);
    UE_NONCOPYABLE(FStartSessionChainedTask);
    virtual ~FStartSessionChainedTask() override = default;

    virtual void ExecuteAsync(const TestUtilities::FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnStartSessionComplete(
        FName CallbackSessionName,
        bool bCallbackWasSuccessful,
        TestUtilities::FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(3102801617, Redpoint::EOS::Tests::Session, FStartSessionChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()