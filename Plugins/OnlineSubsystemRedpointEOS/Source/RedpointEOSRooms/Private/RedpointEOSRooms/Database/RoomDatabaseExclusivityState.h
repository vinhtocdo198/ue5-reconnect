// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3891395109, Redpoint::EOS::Rooms::Database)
{
using namespace ::Redpoint::EOS::Rooms;

class FRoomDatabaseExclusivityState
{
public:
    FRoomDatabaseExclusivityState(const FString &InExclusiveKey);
    UE_NONCOPYABLE(FRoomDatabaseExclusivityState);
    virtual ~FRoomDatabaseExclusivityState() = default;

    // The executing transaction that currently reserves this exclusivity state.
    int ReservedForTransactionId;

    // The pending transactions in the queue that are waiting to acquire this exclusivity state.
    TSet<int> PendingReservationForTransactionIds;

    // If true, this exclusivity state is only being held as part of an auto-leave scenario.
    bool ReservedForLeaveRoom;

    // If true, this exclusivity state will be reserved by a room at the end of the executing transaction.
    bool AssignAtTransactionEnd;

    // If set, this is the room that currently holds this exclusivity key.
    FRoomPtr Room;

    // The exclusivity key this state represents.
    const FString ExclusiveKey;

    // Returns true if the exclusivity state is reserved by an executing transaction or if any of the entries in
    // PendingReservationForTransactionIds have a lower transaction ID than the target transaction.
    FORCEINLINE bool IsReservedByEarlierTransaction(int TargetTransactionId) const
    {
        if (this->ReservedForTransactionId != 0)
        {
            return true;
        }
        if (this->PendingReservationForTransactionIds.Num() == 0)
        {
            return false;
        }
        for (const auto &TransactionId : this->PendingReservationForTransactionIds)
        {
            if (TransactionId < TargetTransactionId)
            {
                return true;
            }
        }
        return false;
    }
};

}

namespace Redpoint::EOS::Rooms::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(3891395109, Redpoint::EOS::Rooms::Database, FRoomDatabaseExclusivityState)
}

REDPOINT_EOS_CODE_GUARD_END()