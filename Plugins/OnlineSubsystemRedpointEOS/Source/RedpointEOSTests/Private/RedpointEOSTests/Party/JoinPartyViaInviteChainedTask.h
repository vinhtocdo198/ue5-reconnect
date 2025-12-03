// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2297956937, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FJoinPartyViaInviteChainedTask : public FChainedTask, public TSharedFromThis<FJoinPartyViaInviteChainedTask>
{
private:
    int32 InstanceId;
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredPartyJoinInfo;

public:
    FJoinPartyViaInviteChainedTask(
        int32 InInstanceId,
        TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> InStoredPartyJoinInfo);
    UE_NONCOPYABLE(FJoinPartyViaInviteChainedTask);
    virtual ~FJoinPartyViaInviteChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnJoinPartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EJoinPartyCompletionResult Result,
        const int32 NotApprovedReason,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(2297956937, Redpoint::EOS::Tests::Party, FJoinPartyViaInviteChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()