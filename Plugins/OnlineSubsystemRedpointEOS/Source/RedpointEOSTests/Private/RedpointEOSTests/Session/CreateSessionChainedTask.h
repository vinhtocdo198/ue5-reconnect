// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSessionSettings.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(882746230, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests;

typedef TDelegate<void(FOnlineSessionSettings &Settings)> FConfigureSessionSettings;

class FCreateSessionChainedTask : public TestUtilities::ChainedTask::FChainedTask,
                                  public TSharedFromThis<FCreateSessionChainedTask>
{
private:
    int32 InstanceId;
    FName SessionName;
    FConfigureSessionSettings ConfigureSessionSettings;
    FDelegateHandle OnCreateSessionCompleteHandle;

public:
    FCreateSessionChainedTask(
        int32 InInstanceId,
        const FName &InSessionName,
        const FConfigureSessionSettings &InConfigureSessionSettings);
    UE_NONCOPYABLE(FCreateSessionChainedTask);
    virtual ~FCreateSessionChainedTask() override = default;

    virtual void ExecuteAsync(const TestUtilities::FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnCreateSessionComplete(
        FName CallbackSessionName,
        bool bCallbackWasSuccessful,
        TestUtilities::FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(882746230, Redpoint::EOS::Tests::Session, FConfigureSessionSettings)
REDPOINT_EOS_FILE_NS_EXPORT(882746230, Redpoint::EOS::Tests::Session, FCreateSessionChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()