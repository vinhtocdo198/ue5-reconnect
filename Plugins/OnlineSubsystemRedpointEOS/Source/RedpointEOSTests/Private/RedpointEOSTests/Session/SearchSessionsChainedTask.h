// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "OnlineSessionSettings.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1721909571, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests;

typedef TDelegate<void(FOnlineSessionSearch &Search)> FConfigureSessionSearch;
typedef TDelegate<bool(const TestUtilities::FChainedTaskContextRef &Context, const FOnlineSessionSearch &Search)>
    FEvaluateSessionSearch;

class FSearchSessionsChainedTask : public TestUtilities::ChainedTask::FChainedTask,
                                   public TSharedFromThis<FSearchSessionsChainedTask>
{
private:
    int32 SearchingInstanceId;
    FConfigureSessionSearch ConfigureSessionSearch;
    FEvaluateSessionSearch EvaluateSessionSearch;
    FDelegateHandle OnFindSessionsCompleteHandle;
    bool bSearchedOnce;

public:
    FSearchSessionsChainedTask(
        int32 InSearchingInstanceId,
        const FConfigureSessionSearch &InConfigureSessionSearch,
        const FEvaluateSessionSearch &InEvaluateSessionSearch);
    UE_NONCOPYABLE(FSearchSessionsChainedTask);
    virtual ~FSearchSessionsChainedTask() override = default;

    virtual void ExecuteAsync(const TestUtilities::FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnFindSessionsComplete(
        bool bCallbackWasSuccessful,
        TSharedRef<FOnlineSessionSearch> Search,
        TestUtilities::FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(1721909571, Redpoint::EOS::Tests::Session, FConfigureSessionSearch)
REDPOINT_EOS_FILE_NS_EXPORT(1721909571, Redpoint::EOS::Tests::Session, FEvaluateSessionSearch)
REDPOINT_EOS_FILE_NS_EXPORT(1721909571, Redpoint::EOS::Tests::Session, FSearchSessionsChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()