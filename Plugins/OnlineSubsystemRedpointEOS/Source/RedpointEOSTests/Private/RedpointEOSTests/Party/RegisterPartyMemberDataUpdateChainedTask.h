// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/PartyMemberDataUpdateStateHandle.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2626929671, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Tests::Party;

class FRegisterPartyMemberDataUpdateChainedTask : public FChainedTask,
                                                  public TSharedFromThis<FRegisterPartyMemberDataUpdateChainedTask>
{
public:
    typedef TDelegate<void(FOnlinePartyData &PartyData)> FModifyPartyData;
    typedef TDelegate<void(const FChainedTaskContextRef &Context, const FOnlinePartyDataConstPtr &PartyData)>
        FVerifyPartyData;

private:
    int32 ListeningInstanceId;
    int32 TargetInstanceId;
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId;
    TSharedRef<FPartyMemberDataUpdateStateHandle> UpdateState;

public:
    FRegisterPartyMemberDataUpdateChainedTask(
        int32 InListeningInstanceId,
        int32 InTargetInstanceId,
        TSharedRef<TSharedPtr<const FOnlinePartyId>> InStoredPartyId,
        TSharedRef<FPartyMemberDataUpdateStateHandle> InUpdateState);
    UE_NONCOPYABLE(FRegisterPartyMemberDataUpdateChainedTask);
    virtual ~FRegisterPartyMemberDataUpdateChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(2626929671, Redpoint::EOS::Tests::Party, FRegisterPartyMemberDataUpdateChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()