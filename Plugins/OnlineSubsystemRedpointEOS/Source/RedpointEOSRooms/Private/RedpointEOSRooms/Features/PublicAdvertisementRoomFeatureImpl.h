// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1095917875, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class FPublicAdvertisementRoomFeatureImpl : public IPublicAdvertisementRoomFeature
{
public:
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1095917875, Redpoint::EOS::Rooms::Features, FPublicAdvertisementRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()