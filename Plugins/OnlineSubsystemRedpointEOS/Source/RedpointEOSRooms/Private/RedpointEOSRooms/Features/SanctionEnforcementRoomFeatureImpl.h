// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(138738348, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class FSanctionEnforcementRoomFeatureImpl : public ISanctionEnforcementRoomFeature
{
public:
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(138738348, Redpoint::EOS::Rooms::Features, FSanctionEnforcementRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()