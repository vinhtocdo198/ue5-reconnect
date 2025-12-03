// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhaseContext.h"

#include "RedpointEOSControlChannel.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(233779596, Redpoint::EOS::Networking::Auth::Login)
{
using namespace ::Redpoint::EOS::Core::Id;

FAuthLoginPhaseContext::FAuthLoginPhaseContext(
    URedpointEOSControlChannel *InControlChannel,
    const UE::Online::FAccountId &InUserId)
    : FAuthPhaseContext(InControlChannel)
    , UserId(InUserId)
{
}

FString FAuthLoginPhaseContext::GetIdentifier() const
{
    return GetUserIdString(this->GetUserId());
}

FString FAuthLoginPhaseContext::GetPhaseGroup() const
{
    return TEXT("login");
}

void FAuthLoginPhaseContext::MarkAsRegisteredForAntiCheat()
{
    URedpointEOSControlChannel *ControlChannelTemp;
    if (GetControlChannel(ControlChannelTemp))
    {
        ControlChannelTemp->bRegisteredForAntiCheat.Add(this->GetUserId(), true);
    }
}

void FAuthLoginPhaseContext::SetVerificationStatus(EUserVerificationStatus InStatus)
{
    URedpointEOSControlChannel *ControlChannelTemp;
    if (GetControlChannel(ControlChannelTemp))
    {
        ControlChannelTemp->VerificationDatabase.Add(this->GetUserId(), InStatus);
    }
}

bool FAuthLoginPhaseContext::GetVerificationStatus(EUserVerificationStatus &OutStatus) const
{
    URedpointEOSControlChannel *ControlChannelTemp;
    if (GetControlChannel(ControlChannelTemp))
    {
        if (!ControlChannelTemp->VerificationDatabase.Contains(this->GetUserId()))
        {
            OutStatus = EUserVerificationStatus::NotStarted;
            return true;
        }

        OutStatus = ControlChannelTemp->VerificationDatabase[this->GetUserId()];
        return true;
    }
    return false;
}

UE::Online::FAccountId FAuthLoginPhaseContext::GetUserId() const
{
    return this->UserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()