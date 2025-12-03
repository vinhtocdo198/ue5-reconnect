// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(673342039, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests;

class FFindSessionByIdChainedTask : public TestUtilities::ChainedTask::FChainedTask,
                                    public TSharedFromThis<FFindSessionByIdChainedTask>
{
private:
    int32 SearchingInstanceId;
    int32 OwningInstanceId;
    FName OwningSessionName;
    TSharedRef<FOnlineSessionSearchResult> FoundSearchResult;
    bool bSearchedOnce;

public:
    FFindSessionByIdChainedTask(
        int32 InSearchingInstanceId,
        int32 InOwningInstanceId,
        FName InOwningSessionName,
        const TSharedRef<FOnlineSessionSearchResult> &InFoundSearchResult);
    UE_NONCOPYABLE(FFindSessionByIdChainedTask);
    virtual ~FFindSessionByIdChainedTask() override = default;

    virtual void ExecuteAsync(const TestUtilities::FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnFindSessionByIdComplete(
        int32 LocalUserId,
        bool bWasSuccessful,
        const FOnlineSessionSearchResult &SearchResult,
        TestUtilities::FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(673342039, Redpoint::EOS::Tests::Session, FFindSessionByIdChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()