// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Synchronisation/SynchronisationContextNull.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2871971520, Redpoint::EOS::Rooms::Synchronisation)
{
using namespace ::Redpoint::EOS::Rooms::Synchronisation;

void FSynchronisationContextNull::AddSynchronisationEvent(
    const FRoomIdRef &InRoomId,
    const FSynchronisationEventRef &InEvent)
{
}

TArray<FSynchronisationEventRef> FSynchronisationContextNull::GetSynchronisationEventOfTypeBase(
    const FRoomIdRef &InRoomId,
    FName InEventType) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return TArray<FSynchronisationEventRef>();
}

void FSynchronisationContextNull::RecordMemberEvictedFromRoom(
    const FRoomIdRef &InRoomId,
    const FRoomUserId &InRoomMemberId)
{
}

TSet<FRoomUserId> FSynchronisationContextNull::GetMembersEvictedFromRoom(const FRoomIdRef &InRoomId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return TSet<FRoomUserId>();
}

void FSynchronisationContextNull::RecordSynchronisationDifference(
    const FRoomRef &InRoom,
    const FSynchronisationDifferenceRef &InSynchronisationDifference)
{
}

TArray<FSynchronisationDifferenceRef> FSynchronisationContextNull::GetSynchronisationDifferences(const FRoomRef &InRoom)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Synchronisation);

    return TArray<FSynchronisationDifferenceRef>();
}

void FSynchronisationContextNull::AddAnticipatedKick(
    const FRoomIdRef &RoomId,
    const FRoomUserId &LocalUserId,
    const FRoomUserId &TargetUserId)
{
}

void FSynchronisationContextNull::RemoveAnticipatedKick(
    const FRoomIdRef &RoomId,
    const FRoomUserId &LocalUserId,
    const FRoomUserId &TargetUserId)

{
}

bool FSynchronisationContextNull::HasAnticipatedKick(
    const FRoomIdRef &RoomId,
    const FRoomUserId &LocalUserId,
    const FRoomUserId &TargetUserId) const
{
    return false;
}

void FSynchronisationContextNull::RemoveAnticipatedKicksExcept(
    const FRoomIdRef &RoomId,
    const FRoomUserId &LocalUserId,
    const TSet<FRoomUserId> &CurrentMembers)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()