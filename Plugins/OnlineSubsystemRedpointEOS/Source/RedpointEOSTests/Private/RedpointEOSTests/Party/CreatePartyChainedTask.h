// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3328965025, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FCreatePartyChainedTask : public FChainedTask, public TSharedFromThis<FCreatePartyChainedTask>
{
private:
    int32 InstanceId;
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId;
    FOnlinePartyTypeId PartyTypeId;
    FDelegateHandle OnPartyJoinedHandle;
    FDelegateHandle OnPartyMemberJoinedHandle;
    bool bSawPartyJoinedEvent;
    bool bSawPartyMemberJoinedEvent;
    TSharedPtr<const FOnlinePartyId> PartyIdFromOnPartyJoined;
    bool bEnableVoiceChat;
    EJoinRequestAction JoinRequestAction;

public:
    FCreatePartyChainedTask(
        int32 InInstanceId,
        TSharedRef<TSharedPtr<const FOnlinePartyId>> InStoredPartyId,
        FOnlinePartyTypeId InPartyTypeId = IOnlinePartySystem::GetPrimaryPartyTypeId(),
        bool bInEnableVoiceChat = false,
        EJoinRequestAction JoinRequestAction = EJoinRequestAction::AutoApprove);
    UE_NONCOPYABLE(FCreatePartyChainedTask);
    virtual ~FCreatePartyChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnPartyCreated(
        const FUniqueNetId &LocalUserId,
        const TSharedPtr<const FOnlinePartyId> &PartyId,
        const ECreatePartyCompletionResult Result,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);
    bool OnNextTick(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone);

    void OnPartyJoinedEvent(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        FChainedTaskContextRef Context);
    void OnPartyMemberJoinedEvent(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        FChainedTaskContextRef Context);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(3328965025, Redpoint::EOS::Tests::Party, FCreatePartyChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()