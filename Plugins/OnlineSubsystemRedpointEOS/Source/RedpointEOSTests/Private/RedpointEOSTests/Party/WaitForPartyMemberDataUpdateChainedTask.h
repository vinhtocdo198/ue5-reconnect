// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/PartyMemberDataUpdateStateHandle.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3390687173, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Tests::Party;

class FWaitForPartyMemberDataUpdateChainedTask : public FChainedTask,
                                                 public TSharedFromThis<FWaitForPartyMemberDataUpdateChainedTask>
{
public:
    typedef TDelegate<void(FOnlinePartyData &PartyData)> FModifyPartyData;
    typedef TDelegate<void(const FChainedTaskContextRef &Context, const FOnlinePartyDataConstPtr &PartyData)>
        FVerifyPartyData;

private:
    TSharedRef<FPartyMemberDataUpdateStateHandle> UpdateState;

public:
    FWaitForPartyMemberDataUpdateChainedTask(TSharedRef<FPartyMemberDataUpdateStateHandle> InUpdateState);
    UE_NONCOPYABLE(FWaitForPartyMemberDataUpdateChainedTask);
    virtual ~FWaitForPartyMemberDataUpdateChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    bool OnTimeout(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(3390687173, Redpoint::EOS::Tests::Party, FWaitForPartyMemberDataUpdateChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()