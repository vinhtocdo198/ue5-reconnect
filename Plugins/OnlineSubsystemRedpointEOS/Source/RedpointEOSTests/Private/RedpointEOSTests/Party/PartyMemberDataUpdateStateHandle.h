// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(202990278, Redpoint::EOS::Tests::Party)
{

class FPartyMemberDataUpdateStateHandle
{
public:
    bool bHandled;
    FDelegateHandle OnPartyDataReceivedHandle;
    FTSTicker::FDelegateHandle OnTimeoutHandle;
    TWeakPtr<IOnlinePartySystem> PartySystem;

    TSharedPtr<const FUniqueNetId> ExpectedLocalUserId;
    TSharedPtr<const FOnlinePartyId> ExpectedPartyId;
    TSharedPtr<const FUniqueNetId> ExpectedMemberId;
    FName ExpectedNamespace;

    TSharedPtr<const FOnlinePartyData> ReceivedPartyData;
    TDelegate<void(bool bContinue)> OnReceivedFunction;

    void OnPartyMemberDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData);
};

}

namespace Redpoint::EOS::Tests::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(202990278, Redpoint::EOS::Tests::Party, FPartyMemberDataUpdateStateHandle)
}

REDPOINT_EOS_CODE_GUARD_END()