// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemTypes.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3162002226, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FVerifyPartyMembershipChainedTask : public FChainedTask, public TSharedFromThis<FVerifyPartyMembershipChainedTask>
{
private:
    int32 InstanceId;
    TArray<TSharedRef<TSharedPtr<const FOnlinePartyId>>> ExpectedPartyIds;

public:
    typedef std::initializer_list<TSharedRef<TSharedPtr<const FOnlinePartyId>>> FInitList;

    FVerifyPartyMembershipChainedTask(
        int32 InInstanceId,
        std::initializer_list<TSharedRef<TSharedPtr<const FOnlinePartyId>>> InExpectedPartyIds);
    UE_NONCOPYABLE(FVerifyPartyMembershipChainedTask);
    virtual ~FVerifyPartyMembershipChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(3162002226, Redpoint::EOS::Tests::Party, FVerifyPartyMembershipChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()