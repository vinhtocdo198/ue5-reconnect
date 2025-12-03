// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomAttribute.h"
#include "RedpointEOSRooms/RoomMember.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(711375327, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;

class FRoomMemberImpl : public IRoomMember
{
private:
    FRoomUserId UserId;
    TMap<FString, FRoomAttribute> RoomMemberAttributes;

public:
    FRoomMemberImpl(const FRoomUserId &InUserId);
    UE_NONCOPYABLE(FRoomMemberImpl);
    virtual ~FRoomMemberImpl() override = default;

    TMap<FString, FRoomAttribute> &GetMutableAttributes();

    virtual const FRoomUserId &GetUserId() const override;
    virtual const TMap<FString, FRoomAttribute> &GetAttributes() const override;
};

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(711375327, Redpoint::EOS::Rooms::Providers, FRoomMemberImpl)
}

REDPOINT_EOS_CODE_GUARD_END()