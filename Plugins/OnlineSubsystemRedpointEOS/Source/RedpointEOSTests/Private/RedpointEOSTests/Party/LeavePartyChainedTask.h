// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "Misc/Optional.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1731051311, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FLeavePartyChainedTask : public FChainedTask, public TSharedFromThis<FLeavePartyChainedTask>
{
private:
    int32 InstanceId;
    TSharedRef<TSharedPtr<const FOnlinePartyId>> PartyId;
    TArray<int32> ObservingInstanceIds;
    TArray<FDelegateHandle> ObservingInstanceExitedDelegateHandles;
    TArray<bool> ObservingInstanceSeenExit;
    TOptional<bool> bLeavePartySucceeded;
    bool bFiredOnDone;

public:
    typedef std::initializer_list<int32> FObserverList;

    FLeavePartyChainedTask(
        int32 InInstanceId,
        TSharedRef<TSharedPtr<const FOnlinePartyId>> InPartyId,
        FObserverList InObservingInstanceIds = {});
    UE_NONCOPYABLE(FLeavePartyChainedTask);
    virtual ~FLeavePartyChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnMemberExited(
        const FUniqueNetId & /*LocalUserId*/,
        const FOnlinePartyId & /*PartyId*/,
        const FUniqueNetId & /*MemberId*/,
        const EMemberExitedReason /*Reason*/,
        int ObserverInstanceIndex,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);

    void OnLeavePartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const ELeavePartyCompletionResult Result,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);

    void CheckReadyForReturn(FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(1731051311, Redpoint::EOS::Tests::Party, FLeavePartyChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()