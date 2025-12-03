// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/UpdatePartyChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1955435195, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;

FUpdatePartyChainedTask::FUpdatePartyChainedTask(
    int32 InInstanceId,
    TSharedRef<TSharedPtr<const FOnlinePartyId>> InStoredPartyId)
    : InstanceId(InInstanceId)
    , StoredPartyId(InStoredPartyId)
{
}

void FUpdatePartyChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    auto PartyInterface = Context->Instance(this->InstanceId).GetPartyInterface();

    auto Party = PartyInterface->GetParty(Context->User(this->InstanceId), **this->StoredPartyId);
    if (!Context->Assert().TestTrue(TEXT("Expected GetParty to return party"), Party.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    if (!PartyInterface->UpdateParty(
            Context->User(this->InstanceId),
            *Party->PartyId,
            *Party->GetConfiguration(),
            false,
            FOnUpdatePartyComplete::CreateSP(this, &FUpdatePartyChainedTask::OnPartyUpdated, Context, OnDone)))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
}

void FUpdatePartyChainedTask::OnPartyUpdated(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const EUpdateConfigCompletionResult Result,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    auto PartyInterface = Context->Instance(this->InstanceId).GetPartyInterface();

    if (!Context->Assert().TestEqual(
            TEXT("Expected UpdateParty operation to succeed."),
            Result,
            EUpdateConfigCompletionResult::Succeeded))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Party system state after updating party:"));
    PartyInterface->DumpPartyState();

    // Return success.
    OnDone.ExecuteIfBound(true);
}

}

REDPOINT_EOS_CODE_GUARD_END()