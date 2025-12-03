// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2179814654, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class FOwnerMigrationRoomFeatureImpl : public IOwnerMigrationRoomFeature
{
public:
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2179814654, Redpoint::EOS::Rooms::Features, FOwnerMigrationRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()