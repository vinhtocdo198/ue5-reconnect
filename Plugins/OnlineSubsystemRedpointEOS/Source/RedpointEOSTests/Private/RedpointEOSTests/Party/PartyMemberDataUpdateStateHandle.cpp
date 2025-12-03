// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/PartyMemberDataUpdateStateHandle.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(202990278, Redpoint::EOS::Tests::Party)
{

void FPartyMemberDataUpdateStateHandle::OnPartyMemberDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const FName &Namespace,
    const FOnlinePartyData &PartyData)
{
    if (LocalUserId == *this->ExpectedLocalUserId && PartyId == *this->ExpectedPartyId &&
        Namespace == this->ExpectedNamespace && MemberId == *this->ExpectedMemberId)
    {
        this->bHandled = true;
        FTSTicker::GetCoreTicker().RemoveTicker(this->OnTimeoutHandle);
        if (auto Pinned = this->PartySystem.Pin())
        {
            Pinned->ClearOnPartyMemberDataReceivedDelegate_Handle(this->OnPartyDataReceivedHandle);
        }
        this->ReceivedPartyData = PartyData.AsShared();
        this->OnReceivedFunction.ExecuteIfBound(true);
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()