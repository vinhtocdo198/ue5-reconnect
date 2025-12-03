// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomLeaveReason.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationEvent.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(151537868, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

class REDPOINTEOSROOMS_API ISynchronisationContext
{
public:
    ISynchronisationContext() = default;
    UE_NONCOPYABLE(ISynchronisationContext);
    virtual ~ISynchronisationContext() = default;

    /**
     * Adds a synchronisation event for this frame. These are constructed from events that the
     * EOS SDK fires, such as "member left" for lobbies.
     */
    virtual void AddSynchronisationEvent(const FRoomIdRef &InRoomId, const FSynchronisationEventRef &InEvent) = 0;

    /**
     * Retrieves all of the synchronisation events for this frame of the specified type. Room
     * synchronisation happens right after the EOS SDK ticks (so this will be populated from
     * EOS SDK events firing). After synchronisation, the list of events is cleared.
     */
    virtual TArray<FSynchronisationEventRef> GetSynchronisationEventOfTypeBase(
        const FRoomIdRef &InRoomId,
        FName InEventType) const = 0;

    /**
     * Retrieves all of the synchronisation events for this frame of the specified type, cast to
     * the specified FSynchronisationEvent<> template.
     */
    template <typename TEventResult>
    TArray<TSharedRef<FSynchronisationEvent<TEventResult>>> GetSynchronisationEventOfType(
        const FRoomIdRef &InRoomId,
        FName InEventType) const
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

        TArray<TSharedRef<FSynchronisationEvent<TEventResult>>> Result;
        for (const auto &Event : GetSynchronisationEventOfTypeBase(InRoomId, InEventType))
        {
            Result.Add(StaticCastSharedRef<FSynchronisationEvent<TEventResult>>(Event));
        }
        return Result;
    }

    /**
     * Retrieves the first synchronisation event for this frame of the specified type, cast to
     * the specified FSynchronisationEvent<> template.
     */
    template <typename TEventResult>
    TSharedPtr<FSynchronisationEvent<TEventResult>> GetFirstSynchronisationEventOfType(
        const FRoomIdRef &InRoomId,
        FName InEventType) const
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

        TArray<TSharedRef<FSynchronisationEvent<TEventResult>>> Result;
        for (const auto &Event : GetSynchronisationEventOfTypeBase(InRoomId, InEventType))
        {
            return StaticCastSharedRef<FSynchronisationEvent<TEventResult>>(Event);
        }
        return nullptr;
    }

    /**
     * Records that a target user has left a room, which may result in the target user no longer being able to
     * read the current room state in the EOS SDK for synchronisation.
     */
    virtual void RecordMemberEvictedFromRoom(const FRoomIdRef &InRoomId, const FRoomUserId &InRoomMemberId) = 0;

    /**
     * Returns a list of user IDs and leave reasons for the room ID.
     */
    virtual TSet<FRoomUserId> GetMembersEvictedFromRoom(const FRoomIdRef &InRoomId) = 0;

    /**
     * Records a synchronisation difference when synchronising this room. Room providers take the list
     * of synchronisation differences after synchronisation and emit engine-level events for the
     * changes in rooms.
     */
    virtual void RecordSynchronisationDifference(
        const FRoomRef &InRoom,
        const FSynchronisationDifferenceRef &InSynchronisationDifference) = 0;

    /**
     * Returns the list of synchronisation differences for the specified room.
     */
    virtual TArray<FSynchronisationDifferenceRef> GetSynchronisationDifferences(const FRoomRef &InRoom) = 0;

    /**
     * Records that the target user might be about to be kicked, until RemoveAnticipatedKick is called.
     */
    virtual void AddAnticipatedKick(
        const FRoomIdRef &RoomId,
        const FRoomUserId &LocalUserId,
        const FRoomUserId &TargetUserId) = 0;

    /**
     * Removes a prior recording of AddAnticipatedKick.
     */
    virtual void RemoveAnticipatedKick(
        const FRoomIdRef &RoomId,
        const FRoomUserId &LocalUserId,
        const FRoomUserId &TargetUserId) = 0;

    /**
     * Returns whether there is an anticipated kick of the target member from the view of the local user.
     */
    virtual bool HasAnticipatedKick(
        const FRoomIdRef &RoomId,
        const FRoomUserId &LocalUserId,
        const FRoomUserId &TargetUserId) const = 0;

    /**
     * Remove any anticipated kicks for the specified room, except for current room members.
     */
    virtual void RemoveAnticipatedKicksExcept(
        const FRoomIdRef &RoomId,
        const FRoomUserId &LocalUserId,
        const TSet<FRoomUserId> &CurrentMembers) = 0;
};

typedef TSharedRef<ISynchronisationContext> FSynchronisationContextRef;
typedef TSharedPtr<ISynchronisationContext> FSynchronisationContextPtr;

}

namespace Redpoint::EOS::Rooms::Synchronisation
{
REDPOINT_EOS_FILE_NS_EXPORT(151537868, Redpoint::EOS::Rooms::Synchronisation, ISynchronisationContext)
REDPOINT_EOS_FILE_NS_EXPORT(151537868, Redpoint::EOS::Rooms::Synchronisation, FSynchronisationContextRef)
REDPOINT_EOS_FILE_NS_EXPORT(151537868, Redpoint::EOS::Rooms::Synchronisation, FSynchronisationContextPtr)
}

REDPOINT_EOS_CODE_GUARD_END()