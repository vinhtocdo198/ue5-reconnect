// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2547115288, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FRoomUpdateInfo
{
public:
    FRoomUpdateInfo()
        : bMaxMembersChanged(false)
        , bOwnerChanged(false)
        , FeaturesUpdated()
        , AttributesAdded()
        , AttributesUpdated()
        , AttributesDeleted()
    {
    }

    bool bMaxMembersChanged;
    bool bOwnerChanged;
    TSet<FName> FeaturesUpdated;
    TSet<FString> AttributesAdded;
    TSet<FString> AttributesUpdated;
    TSet<FString> AttributesDeleted;
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(2547115288, Redpoint::EOS::Rooms, FRoomUpdateInfo)
}

REDPOINT_EOS_CODE_GUARD_END()