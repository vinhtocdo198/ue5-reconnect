// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2147979682, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FRejectAllPendingInvitesChainedTask : public FChainedTask,
                                            public TSharedFromThis<FRejectAllPendingInvitesChainedTask>
{
private:
    int32 InstanceId;
    FTSTicker::FDelegateHandle TimeoutHandle;
    bool bTimedOut;

public:
    FRejectAllPendingInvitesChainedTask(int32 InInstanceId);
    UE_NONCOPYABLE(FRejectAllPendingInvitesChainedTask);
    virtual ~FRejectAllPendingInvitesChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void StartRejection(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);
    bool OnPartyInviteTimeout(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone);
    void OnPartyInviteRemoved(
        const FUniqueNetId &LocalUserId,
        const IOnlinePartyJoinInfo &Invitation,
        EPartyInvitationRemovedReason Reason,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(2147979682, Redpoint::EOS::Tests::Party, FRejectAllPendingInvitesChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()