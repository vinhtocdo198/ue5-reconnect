// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Features/PublicMemberListRoomFeature.h"

#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2382703701, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;

class FPublicMemberListRoomFeatureImpl : public IPublicMemberListRoomFeature
{
public:
    TArray<FRoomUserId> CurrentMembersValue;
    uint32 MaxMembersValue;

    FPublicMemberListRoomFeatureImpl();
    FPublicMemberListRoomFeatureImpl(const TArray<FRoomUserId> &InCurrentMembers, uint32 InMaxMembers);
    UE_NONCOPYABLE(FPublicMemberListRoomFeatureImpl);
    virtual ~FPublicMemberListRoomFeatureImpl() override = default;

    virtual TSharedPtr<IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(2382703701, Redpoint::EOS::Rooms::Features, FPublicMemberListRoomFeatureImpl)
}

REDPOINT_EOS_CODE_GUARD_END()