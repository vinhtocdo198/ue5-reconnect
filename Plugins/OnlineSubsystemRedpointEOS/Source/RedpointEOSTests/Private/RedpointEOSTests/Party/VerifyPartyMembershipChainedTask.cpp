// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSTests/TestUtilities/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3162002226, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

FVerifyPartyMembershipChainedTask::FVerifyPartyMembershipChainedTask(
    int32 InInstanceId,
    std::initializer_list<TSharedRef<TSharedPtr<const FOnlinePartyId>>> InExpectedPartyIds)
    : InstanceId(InInstanceId)
    , ExpectedPartyIds()
{
    for (const auto &ExpectedPartyId : InExpectedPartyIds)
    {
        ExpectedPartyIds.Add(ExpectedPartyId);
    }
}

void FVerifyPartyMembershipChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Party state for instance %d:"), this->InstanceId);
    Context->Instance(this->InstanceId).GetPartyInterface()->DumpPartyState();

    TArray<TSharedRef<const FOnlinePartyId>> PartyIds;
    Context->Assert().TestTrue(
        TEXT("Expected GetJoinedParties to succeed."),
        Context->Instance(this->InstanceId)
            .GetPartyInterface()
            ->GetJoinedParties(Context->User(this->InstanceId), PartyIds));
    Context->Assert().TestEqual(
        FString::Printf(TEXT("Expected to see %d parties, but got %d."), this->ExpectedPartyIds.Num(), PartyIds.Num()),
        PartyIds.Num(),
        this->ExpectedPartyIds.Num());
    for (const auto &ExpectedPartyId : this->ExpectedPartyIds)
    {
        bool bFound = false;
        for (const auto &ActualPartyId : PartyIds)
        {
            if (*ActualPartyId == *ExpectedPartyId.Get())
            {
                bFound = true;
                break;
            }
        }
        Context->Assert().TestTrue(
            FString::Printf(TEXT("Expected to find party ID '%s'."), *ExpectedPartyId.Get()->ToString()),
            bFound);
    }
    OnDone.ExecuteIfBound(true);
}

}

REDPOINT_EOS_CODE_GUARD_END()