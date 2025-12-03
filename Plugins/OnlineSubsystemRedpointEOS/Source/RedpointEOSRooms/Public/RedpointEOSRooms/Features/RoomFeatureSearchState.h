// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(530951227, Redpoint::EOS::Rooms::Features)
{

class REDPOINTEOSROOMS_API IRoomFeatureSearchStateBase
{
public:
    IRoomFeatureSearchStateBase() = default;
    IRoomFeatureSearchStateBase(const IRoomFeatureSearchStateBase &) = default;
    IRoomFeatureSearchStateBase &operator=(const IRoomFeatureSearchStateBase &) = default;
    IRoomFeatureSearchStateBase &operator=(IRoomFeatureSearchStateBase &&) = default;
    virtual ~IRoomFeatureSearchStateBase() = default;

    virtual FName GetFeatureName() const = 0;
};

template <typename TRoomFeature> class IRoomFeatureSearchState : public IRoomFeatureSearchStateBase
{
public:
    IRoomFeatureSearchState() = default;
    IRoomFeatureSearchState(const IRoomFeatureSearchState &) = default;
    IRoomFeatureSearchState &operator=(const IRoomFeatureSearchState &) = default;
    IRoomFeatureSearchState &operator=(IRoomFeatureSearchState &&) = default;
    virtual ~IRoomFeatureSearchState() override = default;

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
REDPOINT_EOS_FILE_NS_EXPORT(530951227, Redpoint::EOS::Rooms::Features, IRoomFeatureSearchStateBase)
REDPOINT_EOS_FILE_NS_EXPORT(530951227, Redpoint::EOS::Rooms::Features, IRoomFeatureSearchState)
}

REDPOINT_EOS_CODE_GUARD_END()