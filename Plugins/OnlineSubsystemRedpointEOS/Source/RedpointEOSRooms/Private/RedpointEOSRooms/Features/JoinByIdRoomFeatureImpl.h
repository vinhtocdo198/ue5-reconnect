// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/JoinByIdRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1022818980, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class FJoinByIdRoomFeatureImpl : public IJoinByIdRoomFeature
{
public:
    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1022818980, Redpoint::EOS::Rooms::Features, FJoinByIdRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()