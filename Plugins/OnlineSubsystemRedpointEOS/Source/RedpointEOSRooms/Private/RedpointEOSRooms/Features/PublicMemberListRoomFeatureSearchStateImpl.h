// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1130234379, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FPublicMemberListRoomFeatureSearchStateImpl : public IPublicMemberListRoomFeatureSearchState
{
public:
    uint32 MaxMembersValue;
    TArray<FRoomUserId> CurrentMembersValue;

    FPublicMemberListRoomFeatureSearchStateImpl();
    UE_NONCOPYABLE(FPublicMemberListRoomFeatureSearchStateImpl);
    virtual ~FPublicMemberListRoomFeatureSearchStateImpl() override = default;

    virtual uint32 MaxMembers() const override;
    virtual uint32 AvailableSlots() const override;
    virtual const TArray<FRoomUserId> &CurrentMembers() const override;
};

}

namespace Redpoint::EOS::Rooms::Features
{
REDPOINT_EOS_FILE_NS_EXPORT(1130234379, Redpoint::EOS::Rooms::Features, FPublicMemberListRoomFeatureSearchStateImpl)
}

REDPOINT_EOS_CODE_GUARD_END()