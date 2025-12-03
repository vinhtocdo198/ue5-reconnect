// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/RegisterPartyMemberDataUpdateChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2626929671, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

FRegisterPartyMemberDataUpdateChainedTask::FRegisterPartyMemberDataUpdateChainedTask(
    int32 InListeningInstanceId,
    int32 InTargetInstanceId,
    TSharedRef<TSharedPtr<const FOnlinePartyId>> InStoredPartyId,
    TSharedRef<FPartyMemberDataUpdateStateHandle> InUpdateState)
    : ListeningInstanceId(InListeningInstanceId)
    , TargetInstanceId(InTargetInstanceId)
    , StoredPartyId(InStoredPartyId)
    , UpdateState(InUpdateState)
{
}

void FRegisterPartyMemberDataUpdateChainedTask::ExecuteAsync(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    this->UpdateState->ExpectedLocalUserId = Context->User(this->ListeningInstanceId).AsShared();
    this->UpdateState->ExpectedPartyId = *this->StoredPartyId;
    this->UpdateState->ExpectedNamespace = DefaultPartyDataNamespace;
    this->UpdateState->ExpectedMemberId = Context->User(this->TargetInstanceId).AsShared();
    this->UpdateState->PartySystem = Context->Instance(this->ListeningInstanceId).GetPartyInterface();
    this->UpdateState->bHandled = false;
    this->UpdateState->OnTimeoutHandle.Reset();
    this->UpdateState->OnPartyDataReceivedHandle =
        Context->Instance(this->ListeningInstanceId)
            .GetPartyInterface()
            ->AddOnPartyMemberDataReceivedDelegate_Handle(FOnPartyMemberDataReceived::FDelegate::CreateSP(
                this->UpdateState,
                &FPartyMemberDataUpdateStateHandle::OnPartyMemberDataReceived));
    OnDone.ExecuteIfBound(true);
}

}

REDPOINT_EOS_CODE_GUARD_END()