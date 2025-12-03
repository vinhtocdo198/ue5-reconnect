// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2289569929, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPartyTypeRoomFeatureSearchStateImpl : public IPartyTypeRoomFeatureSearchState
{
public:
    uint32 PartyTypeIdValue;

    FPartyTypeRoomFeatureSearchStateImpl();
    UE_NONCOPYABLE(FPartyTypeRoomFeatureSearchStateImpl);
    virtual ~FPartyTypeRoomFeatureSearchStateImpl() override = default;

    virtual uint32 PartyTypeId() const override;
    virtual bool PartyTypeIndicatesPresenceEnabled() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2289569929, Redpoint::EOS::Rooms::Features, FPartyTypeRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()