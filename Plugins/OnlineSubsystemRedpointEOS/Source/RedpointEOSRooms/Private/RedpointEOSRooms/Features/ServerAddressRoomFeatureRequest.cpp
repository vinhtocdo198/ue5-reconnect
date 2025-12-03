// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/ServerAddressRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/ServerAddressRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3646340146, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FServerAddressRoomFeatureRequest::FServerAddressRoomFeatureRequest(
    EServerAddressMode InAddressMode,
    const FString &InAddress)
    : AddressModeValue(InAddressMode)
    , AddressValue(InAddress)
{
}

FServerAddressRoomFeatureRequest::EServerAddressMode FServerAddressRoomFeatureRequest::AddressMode() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->AddressModeValue;
}

const FString &FServerAddressRoomFeatureRequest::Address() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->AddressValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()