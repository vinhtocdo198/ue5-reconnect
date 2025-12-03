// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2396472480, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

extern REDPOINTEOSROOMS_API const uint32 PartyTypePrimaryPartyId;

class REDPOINTEOSROOMS_API FPartyTypeRoomFeatureRequest : public IRoomFeatureRequest<IPartyTypeRoomFeature>
{
private:
    uint32 PartyTypeIdValue;

public:
    FPartyTypeRoomFeatureRequest(uint32 InPartyTypeId);
    UE_NONCOPYABLE(FPartyTypeRoomFeatureRequest);
    virtual ~FPartyTypeRoomFeatureRequest() override = default;

    virtual bool IsValid() const override;
    uint32 PartyTypeId() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2396472480, Redpoint::EOS::Rooms::Features, PartyTypePrimaryPartyId)
REDPOINT_EOS_FILE_NS_EXPORT(2396472480, Redpoint::EOS::Rooms::Features, FPartyTypeRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()