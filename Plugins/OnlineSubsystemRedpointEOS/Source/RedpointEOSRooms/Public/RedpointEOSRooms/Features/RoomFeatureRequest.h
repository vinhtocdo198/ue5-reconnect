// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomAttribute.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(2575674398, Redpoint::EOS::Rooms, IRoomSearchResult);

namespace REDPOINT_EOS_FILE_NS_ID(1465771690, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API IRoomFeatureRequestBase
{
public:
    IRoomFeatureRequestBase() = default;
    IRoomFeatureRequestBase(const IRoomFeatureRequestBase &) = default;
    IRoomFeatureRequestBase &operator=(const IRoomFeatureRequestBase &) = default;
    IRoomFeatureRequestBase &operator=(IRoomFeatureRequestBase &&) = default;
    virtual ~IRoomFeatureRequestBase() = default;

    virtual FName GetFeatureName() const = 0;
    virtual bool IsValid() const
    {
        return true;
    }
};

template <typename TRoomFeature> class IRoomFeatureRequest : public IRoomFeatureRequestBase
{
public:
    IRoomFeatureRequest() = default;
    IRoomFeatureRequest(const IRoomFeatureRequest &) = default;
    IRoomFeatureRequest &operator=(const IRoomFeatureRequest &) = default;
    IRoomFeatureRequest &operator=(IRoomFeatureRequest &&) = default;
    virtual ~IRoomFeatureRequest() override = default;

    static FName GetFeatureNameStatic()
    {
        return TRoomFeature::GetFeatureName();
    }
    virtual FName GetFeatureName() const override
    {
        return TRoomFeature::GetFeatureName();
    }
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1465771690, Redpoint::EOS::Rooms::Features, IRoomFeatureRequestBase)
REDPOINT_EOS_FILE_NS_EXPORT(1465771690, Redpoint::EOS::Rooms::Features, IRoomFeatureRequest)
}

REDPOINT_EOS_CODE_GUARD_END()