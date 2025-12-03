// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Verification/AuthVerificationPhaseContext.h"

#include "RedpointEOSControlChannel.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1384477030, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::Core::Id;

FAuthVerificationPhaseContext::FAuthVerificationPhaseContext(
    URedpointEOSControlChannel *InControlChannel,
    const UE::Online::FAccountId &InUserId)
    : FAuthPhaseContext(InControlChannel)
    , UserId(InUserId)
{
}

FString FAuthVerificationPhaseContext::GetIdentifier() const
{
    return GetUserIdString(this->GetUserId());
}

FString FAuthVerificationPhaseContext::GetPhaseGroup() const
{
    return TEXT("verification");
}

void FAuthVerificationPhaseContext::SetVerificationStatus(EUserVerificationStatus InStatus)
{
    URedpointEOSControlChannel *ControlChannelTemp;
    if (this->GetControlChannel(ControlChannelTemp))
    {
        ControlChannelTemp->VerificationDatabase.Add(this->GetUserId(), InStatus);
    }
}

bool FAuthVerificationPhaseContext::GetVerificationStatus(EUserVerificationStatus &OutStatus) const
{
    URedpointEOSControlChannel *ControlChannelTemp;
    if (this->GetControlChannel(ControlChannelTemp))
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

bool FAuthVerificationPhaseContext::IsConnectionAsTrustedOnClient(bool &OutIsTrusted) const
{
    URedpointEOSControlChannel *ControlChannelTemp;
    if (this->GetControlChannel(ControlChannelTemp))
    {
        OutIsTrusted = ControlChannelTemp->bClientTrustsServer;
        return true;
    }
    return false;
}

UE::Online::FAccountId FAuthVerificationPhaseContext::GetUserId() const
{
    return this->UserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()