// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/MatchStateRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3686280070, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class FMatchStateRoomFeatureImpl : public IMatchStateRoomFeature
{
public:
    virtual void StartMatchInRoom(
        const FStartMatchInRoomRequest &Request,
        const FOnStartMatchInRoomComplete &OnComplete) override;
    virtual void EndMatchInRoom(const FEndMatchInRoomRequest &Request, const FOnEndMatchInRoomComplete &OnComplete)
        override;

    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3686280070, Redpoint::EOS::Rooms::Features, FMatchStateRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()