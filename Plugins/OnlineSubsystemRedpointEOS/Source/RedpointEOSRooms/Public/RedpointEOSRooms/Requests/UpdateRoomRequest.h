// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/Optional.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/FeatureRequestsOnRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2218672566, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;

class REDPOINTEOSROOMS_API FUpdateRoomRequest : public FFeatureRequestsOnRequest
{
protected:
    /**
     * The features to entirely remove from the room.
     */
    TSet<FName> DeleteFeatureNames;

public:
    /**
     * The new maximum number of members to permit in the room.
     */
    TOptional<uint32> MaxMemberCount;

    /**
     * The publicly visible attributes to set or update on the room.
     */
    TMap<FString, FRoomAttribute> UpsertAttributes;

    /**
     * The publicly visible attributes to remove from the room.
     */
    TSet<FString> DeleteAttributes;

    /**
     * The publicly visible attributes to set or update on the current member of this room.
     */
    TMap<FString, FRoomAttribute> UpsertMemberAttributes;

    /**
     * The publicly visible attributes to remove on the current member of this room.
     */
    TSet<FString> DeleteMemberAttributes;

    /**
     * Returns the names of the features to removed from this room.
     */
    const TSet<FName> GetDeleteFeatureNames() const;

    /**
     * Schedules a feature to be removed from the room.
     */
    void DeleteFeature(FName FeatureName);

    /**
     * Removes an update to a feature that has previously been scheduled with AddFeatureRequest.
     */
    void RevertFeature(FName FeatureName);
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(2218672566, Redpoint::EOS::Rooms::Requests, FUpdateRoomRequest)
}

REDPOINT_EOS_CODE_GUARD_END()