// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomInvite.h"
#include "RedpointEOSRooms/RoomRequestedJoin.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4026645771, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FRoomRequestedJoinImpl : public IRoomRequestedJoin
{
private:
    FRoomUserId ActorUserId;
    FRoomSearchResultRef RoomSearchResult;
    FRoomInvitePtr RoomInvite;

public:
    FRoomRequestedJoinImpl(const FRoomUserId &InActorUserId, const FRoomSearchResultRef &InRoomSearchResult);
    FRoomRequestedJoinImpl(const FRoomInviteRef &InRoomInvite);
    UE_NONCOPYABLE(FRoomRequestedJoinImpl);
    virtual ~FRoomRequestedJoinImpl() override = default;

    virtual FRoomUserId GetActorUserId() const override;
    virtual FRoomSearchResultRef GetRoomSearchResult() const override;

    FRoomInvitePtr GetRoomInvite() const;
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(4026645771, Redpoint::EOS::Rooms, FRoomRequestedJoinImpl)
}

REDPOINT_EOS_CODE_GUARD_END()