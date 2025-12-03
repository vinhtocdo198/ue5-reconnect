// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationDifference.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3146034446, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API FRoomMemberSynchronisationDifference : public ISynchronisationDifference
{
public:
    enum class EChangeType : uint8
    {
        Joined,
        Left,
        Disconnected,
        Kicked,
        // @note: We represent this difference as RoomOwnerSynchronisationDifference.
        // Promoted,
        Closed,
    };

private:
    FRoomUserId MemberUserId;
    EChangeType ChangeType;
    bool bIsForBoundUser;

public:
    FRoomMemberSynchronisationDifference(FRoomUserId InMemberUserId, EChangeType InChangeType, bool InIsForBoundUser);
    UE_NONCOPYABLE(FRoomMemberSynchronisationDifference);
    virtual ~FRoomMemberSynchronisationDifference() override = default;

    static FName GetDifferenceTypeStatic();
    virtual FName GetDifferenceType() const override;
    FRoomUserId GetMemberUserId() const;
    EChangeType GetChangeType() const;
    bool IsForBoundUser() const;
};

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(3146034446, Redpoint::EOS::Rooms::Synchronisation, FRoomMemberSynchronisationDifference)
}

REDPOINT_EOS_CODE_GUARD_END()