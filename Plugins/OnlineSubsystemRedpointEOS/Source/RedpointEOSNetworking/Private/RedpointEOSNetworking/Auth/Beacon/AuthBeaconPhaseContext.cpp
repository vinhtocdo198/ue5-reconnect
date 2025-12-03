// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Beacon/AuthBeaconPhaseContext.h"

#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3050450103, Redpoint::EOS::Networking::Auth::Beacon)
{
using namespace ::Redpoint::EOS::Core::Id;

FString FAuthBeaconPhaseContext::GetIdentifier() const
{
    return FString::Printf(TEXT("%s/%s"), *GetUserIdString(this->GetUserId()), *this->BeaconName);
}

FString FAuthBeaconPhaseContext::GetPhaseGroup() const
{
    return TEXT("beacon");
}

FAuthBeaconPhaseContext::FAuthBeaconPhaseContext(
    URedpointEOSControlChannel *InControlChannel,
    UE::Online::FAccountId InUserId,
    const FString &InBeaconName)
    : FAuthPhaseContext(InControlChannel)
    , UserId(InUserId)
    , BeaconName(InBeaconName)
{
}

UE::Online::FAccountId FAuthBeaconPhaseContext::GetUserId() const
{
    return this->UserId;
}

FString FAuthBeaconPhaseContext::GetBeaconName() const
{
    return this->BeaconName;
}

}

REDPOINT_EOS_CODE_GUARD_END()