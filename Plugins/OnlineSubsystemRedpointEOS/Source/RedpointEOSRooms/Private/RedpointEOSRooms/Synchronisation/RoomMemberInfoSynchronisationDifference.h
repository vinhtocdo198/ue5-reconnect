// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationDifference.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(983476630, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API FRoomMemberInfoSynchronisationDifference : public ISynchronisationDifference
{
private:
    FRoomUserId MemberUserId;

public:
    FRoomMemberInfoSynchronisationDifference(FRoomUserId InMemberUserId);
    UE_NONCOPYABLE(FRoomMemberInfoSynchronisationDifference);
    virtual ~FRoomMemberInfoSynchronisationDifference() override = default;

    static FName GetDifferenceTypeStatic();
    virtual FName GetDifferenceType() const override;
    FRoomUserId GetMemberUserId() const;
};

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(983476630, Redpoint::EOS::Rooms::Synchronisation, FRoomMemberInfoSynchronisationDifference)
}

REDPOINT_EOS_CODE_GUARD_END()