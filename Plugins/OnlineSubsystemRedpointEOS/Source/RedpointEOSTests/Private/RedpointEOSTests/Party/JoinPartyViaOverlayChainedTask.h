// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(621343221, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FJoinPartyViaOverlayChainedTask : public FChainedTask, public TSharedFromThis<FJoinPartyViaOverlayChainedTask>
{
private:
    int32 InstanceId;
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredPartyJoinInfo;
    bool bAutoApprove;
    bool bShouldApprove;
    FDelegateHandle OnPartyJoinedDelegateHandle;
    FDelegateHandle OnPartyRequestToJoinReceivedDelegateHandle;
    FTSTicker::FDelegateHandle TimerHandle;

public:
    FJoinPartyViaOverlayChainedTask(
        int32 InInstanceId,
        TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> InStoredPartyJoinInfo,
        bool bInAutoApprove,
        bool bInShouldApprove);
    UE_NONCOPYABLE(FJoinPartyViaOverlayChainedTask);
    virtual ~FJoinPartyViaOverlayChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    bool OnTimeout(float, FChainedTaskContextRef Context, FOnComplete OnDone);

    void OnPartyJoined(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);

    void OnPartyRequestToJoinReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &RequesterId,
        const IOnlinePartyRequestToJoinInfo &Request,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(621343221, Redpoint::EOS::Tests::Party, FJoinPartyViaOverlayChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()