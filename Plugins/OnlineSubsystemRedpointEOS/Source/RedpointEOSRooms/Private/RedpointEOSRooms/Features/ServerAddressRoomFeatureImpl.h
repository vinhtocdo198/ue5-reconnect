// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/ServerAddressRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1983455148, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class FServerAddressRoomFeatureImpl : public IServerAddressRoomFeature
{
public:
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1983455148, Redpoint::EOS::Rooms::Features, FServerAddressRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()