// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3267409433, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSROOMS_API IMatchStateRoomFeature : public IRoomFeature
{
public:
    IMatchStateRoomFeature() = default;
    IMatchStateRoomFeature(const IMatchStateRoomFeature &) = default;
    IMatchStateRoomFeature &operator=(const IMatchStateRoomFeature &) = default;
    IMatchStateRoomFeature &operator=(IMatchStateRoomFeature &&) = default;
    virtual ~IMatchStateRoomFeature() override = default;

    typedef TDelegate<void(FError ErrorCode)> FOnStartMatchInRoomComplete;
    typedef TDelegate<void(FError ErrorCode)> FOnEndMatchInRoomComplete;

    static FName GetFeatureName();

    class REDPOINTEOSROOMS_API FStartMatchInRoomRequest
    {
    };

    /**
     * Starts the match in the room this feature is associated with.
     */
    virtual void StartMatchInRoom(
        const FStartMatchInRoomRequest &Request,
        const FOnStartMatchInRoomComplete &OnComplete) = 0;

    class REDPOINTEOSROOMS_API FEndMatchInRoomRequest
    {
    };

    /**
     * Ends the match in the room this feature is associated with.
     */
    virtual void EndMatchInRoom(const FEndMatchInRoomRequest &Request, const FOnEndMatchInRoomComplete &OnComplete) = 0;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(3267409433, Redpoint::EOS::Rooms::Features, IMatchStateRoomFeature)
}

REDPOINT_EOS_CODE_GUARD_END()