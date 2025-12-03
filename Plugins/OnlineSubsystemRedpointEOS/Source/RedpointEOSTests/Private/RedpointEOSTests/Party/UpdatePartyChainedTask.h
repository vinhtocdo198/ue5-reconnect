// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1955435195, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FUpdatePartyChainedTask : public FChainedTask, public TSharedFromThis<FUpdatePartyChainedTask>
{
private:
    int32 InstanceId;
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId;

public:
    FUpdatePartyChainedTask(int32 InInstanceId, TSharedRef<TSharedPtr<const FOnlinePartyId>> InStoredPartyId);
    UE_NONCOPYABLE(FUpdatePartyChainedTask);
    virtual ~FUpdatePartyChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnPartyUpdated(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EUpdateConfigCompletionResult Result,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(1955435195, Redpoint::EOS::Tests::Party, FUpdatePartyChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()