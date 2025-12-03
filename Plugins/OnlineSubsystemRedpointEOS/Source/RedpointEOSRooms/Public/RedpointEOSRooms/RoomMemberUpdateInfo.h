// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3925297258, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FRoomMemberUpdateInfo
{
public:
    FRoomMemberUpdateInfo()
        : bInfoUpdated(false)
        , AttributesAdded()
        , AttributesUpdated()
        , AttributesDeleted()
    {
    }

    bool bInfoUpdated;
    TSet<FString> AttributesAdded;
    TSet<FString> AttributesUpdated;
    TSet<FString> AttributesDeleted;
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(3925297258, Redpoint::EOS::Rooms, FRoomMemberUpdateInfo)
}

REDPOINT_EOS_CODE_GUARD_END()