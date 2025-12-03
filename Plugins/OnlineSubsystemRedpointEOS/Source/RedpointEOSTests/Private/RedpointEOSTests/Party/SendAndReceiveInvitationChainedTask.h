// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1587078365, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FSendAndReceiveInvitationChainedTask : public FChainedTask,
                                             public TSharedFromThis<FSendAndReceiveInvitationChainedTask>
{
private:
    int32 FromInstanceId;
    int32 ToInstanceId;
    TSharedRef<TSharedPtr<const FOnlinePartyId>> PartyId;
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredPartyJoinInfo;
    FDelegateHandle OnPartyInviteReceivedHandle;
    FTSTicker::FDelegateHandle TimeoutHandle;
    bool bTimedOut;

public:
    FSendAndReceiveInvitationChainedTask(
        int32 InFromInstanceId,
        int32 InToInstanceId,
        TSharedRef<TSharedPtr<const FOnlinePartyId>> InPartyId,
        TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> InStoredPartyJoinInfo);
    UE_NONCOPYABLE(FSendAndReceiveInvitationChainedTask);
    virtual ~FSendAndReceiveInvitationChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnPartyInviteReceived(
        const FUniqueNetId &LocalUserId,
        const IOnlinePartyJoinInfo &Invitation,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
    bool OnPartyInviteTimeout(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(1587078365, Redpoint::EOS::Tests::Party, FSendAndReceiveInvitationChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()