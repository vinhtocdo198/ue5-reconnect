// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3938274349, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FSetPartyDataChainedTask : public FChainedTask, public TSharedFromThis<FSetPartyDataChainedTask>
{
public:
    typedef TDelegate<void(FOnlinePartyData &PartyData)> FModifyPartyData;
    typedef TDelegate<void(const FChainedTaskContextRef &Context, const FOnlinePartyDataConstPtr &PartyData)>
        FVerifyPartyData;

private:
    int32 InstanceId;
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId;
    FModifyPartyData ModifyPartyData;
    FVerifyPartyData VerifyPartyData;
    bool bIsMemberData;
    FDelegateHandle EventHandle;
    FTSTicker::FDelegateHandle TimeoutHandle;
    bool bHandled;

public:
    FSetPartyDataChainedTask(
        int32 InInstanceId,
        TSharedRef<TSharedPtr<const FOnlinePartyId>> InStoredPartyId,
        FModifyPartyData InModifyPartyData,
        FVerifyPartyData InVerifyPartyData,
        bool bInIsMemberData);
    UE_NONCOPYABLE(FSetPartyDataChainedTask);
    virtual ~FSetPartyDataChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    bool OnTimeout(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone);
    void OnPartyDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
    void OnPartyMemberDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(3938274349, Redpoint::EOS::Tests::Party, FSetPartyDataChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()