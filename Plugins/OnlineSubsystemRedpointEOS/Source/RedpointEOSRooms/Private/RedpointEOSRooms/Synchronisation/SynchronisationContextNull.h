// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2871971520, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

/**
 * This acts like an ISynchronisationContext, but just consumes all recorded synchronisation events and
 * discards them. It is intended to be used when room providers need to perform an initial synchronisation
 * of the room state and don't want to fire engine events in response to the initialisation.
 */
class REDPOINTEOSROOMS_API FSynchronisationContextNull : public ISynchronisationContext
{
public:
    FSynchronisationContextNull() = default;
    UE_NONCOPYABLE(FSynchronisationContextNull);
    virtual ~FSynchronisationContextNull() override = default;

    virtual void AddSynchronisationEvent(const FRoomIdRef &InRoomId, const FSynchronisationEventRef &InEvent) override;
    virtual TArray<FSynchronisationEventRef> GetSynchronisationEventOfTypeBase(
        const FRoomIdRef &InRoomId,
        FName InEventType) const override;
    virtual void RecordMemberEvictedFromRoom(const FRoomIdRef &InRoomId, const FRoomUserId &InRoomMemberId) override;
    virtual TSet<FRoomUserId> GetMembersEvictedFromRoom(const FRoomIdRef &InRoomId) override;
    virtual void RecordSynchronisationDifference(
        const FRoomRef &InRoom,
        const FSynchronisationDifferenceRef &InSynchronisationDifference) override;
    virtual TArray<FSynchronisationDifferenceRef> GetSynchronisationDifferences(const FRoomRef &InRoom) override;
    virtual void AddAnticipatedKick(
        const FRoomIdRef &RoomId,
        const FRoomUserId &LocalUserId,
        const FRoomUserId &TargetUserId) override;
    virtual void RemoveAnticipatedKick(
        const FRoomIdRef &RoomId,
        const FRoomUserId &LocalUserId,
        const FRoomUserId &TargetUserId) override;
    virtual bool HasAnticipatedKick(
        const FRoomIdRef &RoomId,
        const FRoomUserId &LocalUserId,
        const FRoomUserId &TargetUserId) const override;
    virtual void RemoveAnticipatedKicksExcept(
        const FRoomIdRef &RoomId,
        const FRoomUserId &LocalUserId,
        const TSet<FRoomUserId> &CurrentMembers) override;
};

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(2871971520, Redpoint::EOS::Rooms::Synchronisation, FSynchronisationContextNull)
}

REDPOINT_EOS_CODE_GUARD_END()