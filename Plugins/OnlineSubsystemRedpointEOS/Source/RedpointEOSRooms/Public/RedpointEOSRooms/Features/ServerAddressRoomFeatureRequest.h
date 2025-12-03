// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/ServerAddressRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3646340146, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FServerAddressRoomFeatureRequest : public IRoomFeatureRequest<IServerAddressRoomFeature>
{
public:
    enum class EServerAddressMode : uint8
    {
        /**
         * The server address is automatically determined by the EOS backend.
         */
        Automatic = 0,

        /**
         * The server address specified by this feature is used.
         */
        Manual = 1,
    };

private:
    EServerAddressMode AddressModeValue;
    FString AddressValue;

public:
    FServerAddressRoomFeatureRequest(EServerAddressMode InAddressMode, const FString &InAddress);
    UE_NONCOPYABLE(FServerAddressRoomFeatureRequest);
    virtual ~FServerAddressRoomFeatureRequest() override = default;

    EServerAddressMode AddressMode() const;
    const FString &Address() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3646340146, Redpoint::EOS::Rooms::Features, FServerAddressRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()