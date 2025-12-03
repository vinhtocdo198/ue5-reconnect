// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/AliasRoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(668254285, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FAliasRoomFeatureRequest : public IRoomFeatureRequest<IAliasRoomFeature>
{
private:
    FName AliasValue;

public:
    FAliasRoomFeatureRequest(const FName &InAlias);
    UE_NONCOPYABLE(FAliasRoomFeatureRequest);
    virtual ~FAliasRoomFeatureRequest() override = default;

    virtual bool IsValid() const override;
    const FName &Alias() const;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(668254285, Redpoint::EOS::Rooms::Features, FAliasRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()