// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "OnlineSessionSettings.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Session/CreateSessionChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3329562986, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests;
using namespace ::Redpoint::EOS::Tests::Session;

class FUpdateSessionChainedTask : public TestUtilities::ChainedTask::FChainedTask,
                                  public TSharedFromThis<FUpdateSessionChainedTask>
{
private:
    int32 InstanceId;
    FName SessionName;
    FConfigureSessionSettings ConfigureSessionSettings;
    FDelegateHandle OnUpdateSessionCompleteHandle;

public:
    FUpdateSessionChainedTask(
        int32 InInstanceId,
        const FName &InSessionName,
        const FConfigureSessionSettings &InConfigureSessionSettings);
    UE_NONCOPYABLE(FUpdateSessionChainedTask);
    virtual ~FUpdateSessionChainedTask() override = default;

    virtual void ExecuteAsync(const TestUtilities::FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnUpdateSessionComplete(
        FName CallbackSessionName,
        bool bCallbackWasSuccessful,
        TestUtilities::FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(3329562986, Redpoint::EOS::Tests::Session, FUpdateSessionChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()