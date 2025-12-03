// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomInvite.h"
#include "RedpointEOSRooms/RoomInviteId.h"
#include "RedpointEOSRooms/RoomSearchResult.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(529624270, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;

class FRoomInviteImpl : public IRoomInvite
{
protected:
    FRoomInviteId RoomInviteId;
    FRoomUserId RoomInviteSenderUserId;
    FRoomUserId RoomInviteRecipientUserId;
    FRoomSearchResultRef RoomSearchResult;

public:
    FRoomInviteImpl(
        const FRoomInviteId &InRoomInviteId,
        const FRoomUserId &InRoomInviteSenderUserId,
        const FRoomUserId &InRoomInviteRecipientUserId,
        const FRoomSearchResultRef &InRoomSearchResult);
    UE_NONCOPYABLE(FRoomInviteImpl);
    virtual ~FRoomInviteImpl() override = default;

    virtual FString GetRoomInviteId() const override;
    virtual FRoomUserId GetRoomInviteSenderUserId() const override;
    virtual FRoomUserId GetRoomInviteRecipientUserId() const override;
    virtual TSharedRef<IRoomSearchResult> GetRoomInviteSearchResult() const override;
};

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(529624270, Redpoint::EOS::Rooms::Providers, FRoomInviteImpl)
}

REDPOINT_EOS_CODE_GUARD_END()