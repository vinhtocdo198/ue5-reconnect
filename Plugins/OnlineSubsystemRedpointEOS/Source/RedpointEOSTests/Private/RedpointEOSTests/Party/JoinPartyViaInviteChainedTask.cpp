// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2297956937, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

FJoinPartyViaInviteChainedTask::FJoinPartyViaInviteChainedTask(
    int32 InInstanceId,
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> InStoredPartyJoinInfo)
    : InstanceId(InInstanceId)
    , StoredPartyJoinInfo(InStoredPartyJoinInfo)
{
}

void FJoinPartyViaInviteChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    if (Context->Assert().TestTrue(TEXT("Expected to have party info to join!"), this->StoredPartyJoinInfo->IsValid()))
    {
        Context->Instance(this->InstanceId)
            .GetPartyInterface()
            ->JoinParty(
                Context->User(this->InstanceId),
                **this->StoredPartyJoinInfo,
                FOnJoinPartyComplete::CreateSP(
                    this,
                    &FJoinPartyViaInviteChainedTask::OnJoinPartyComplete,
                    Context,
                    OnDone));
    }
    else
    {
        OnDone.ExecuteIfBound(false);
    }
}

void FJoinPartyViaInviteChainedTask::OnJoinPartyComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const EJoinPartyCompletionResult Result,
    const int32 NotApprovedReason,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    OnDone.ExecuteIfBound(Context->Assert().TestEqual(
        TEXT("Expected EJoinPartyCompletionResult to be successful."),
        Result,
        EJoinPartyCompletionResult::Succeeded));
}

}

REDPOINT_EOS_CODE_GUARD_END()