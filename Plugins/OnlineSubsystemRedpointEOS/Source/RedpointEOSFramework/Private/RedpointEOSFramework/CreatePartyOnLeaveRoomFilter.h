// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSRooms/Filters/RoomFilter.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3198079518, Redpoint::EOS::Framework)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Filters;
using namespace ::Redpoint::EOS::Rooms::Requests;

class FCreatePartyOnLeaveRoomFilter : public IRoomFilter, public TSharedFromThis<FCreatePartyOnLeaveRoomFilter>
{
public:
    FCreatePartyOnLeaveRoomFilter() = default;
    UE_NONCOPYABLE(FCreatePartyOnLeaveRoomFilter);
    virtual ~FCreatePartyOnLeaveRoomFilter() override = default;

    virtual FString GetFilterDebugName() const override
    {
        return TEXT("CreatePartyOnLeaveRoomFilter");
    }

    virtual ERoomFilterAction OnLeaveRoom(
        const FPlatformHandle &PlatformHandle,
        const FRoomRef &Room,
        FLeaveRoomRequest &Request,
        const IRoomSystem::FOnLeaveRoomComplete &OnComplete) override;

    virtual void OnRoomLeftExternally(
        const FPlatformHandle &PlatformHandle,
        const FRoomRef &Room,
        ERoomLeaveReason RoomLeaveReason) override;
};

}

namespace Redpoint::EOS::Framework
{
REDPOINT_EOS_FILE_NS_EXPORT(3198079518, Redpoint::EOS::Framework, FCreatePartyOnLeaveRoomFilter)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()