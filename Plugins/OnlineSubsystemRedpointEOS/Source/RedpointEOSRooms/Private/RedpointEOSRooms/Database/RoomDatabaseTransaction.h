// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Database/RoomDatabaseExclusivityState.h"
#include "RedpointEOSRooms/RoomScope.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2834721303, Redpoint::EOS::Rooms::Database)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Database;

class FRoomDatabaseTransaction
{
public:
    FRoomDatabaseTransaction(int InTransactionId, ERoomScope InRoomScope, FRoomUserId InLocalUserId);
    UE_NONCOPYABLE(FRoomDatabaseTransaction);

    // @note: We don't checkf() that DeferredEventBroadcasts is empty here, because it's possible for the room database
    // (and it's transactions) to be destructed when the game is shutting down while there are still transactions in
    // progress.
    ~FRoomDatabaseTransaction() = default;

    const int TransactionId;
    const ERoomScope RoomScope;
    const FRoomUserId LocalUserId;

    // If true, this transaction is currently executing. If false, this transaction is pending (blocked by another
    // transaction) or about to fire.
    bool bExecuting;

    // The set of other transaction IDs that currently block this transaction from starting execution.
    TSet<int> WaitingOnTransactionIds;

    // The exclusivity states acquired by this transaction at the point when it starts executing.
    TSet<TSharedRef<FRoomDatabaseExclusivityState>> ExclusivityStates;

    // Mark this transaction as having started execution with the set of exclusivity states acquired.
    void StartExecution(const TSet<TSharedRef<FRoomDatabaseExclusivityState>> &InExclusivityStates);

    /**
     * When the room system wants to broadcast an event (typically in response to synchronisation), we have to defer the
     * event broadcast until there are no transactions holding exclusive keys that belong to the relevant room. This
     * ensures that user code that might be listening for those events won't start operations while there are already
     * transactions in progress.
     */
    TArray<FSimpleDelegate> DeferredEventBroadcasts;

    // The callback to execute when this transaction might be ready to start again. If the transaction isn't ready to
    // start when this callback runs, it must run the deferral logic again.
    FSimpleDelegate ContinueAfterDeferral;
};

}

namespace Redpoint::EOS::Rooms::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(2834721303, Redpoint::EOS::Rooms::Database, FRoomDatabaseTransaction)
}

REDPOINT_EOS_CODE_GUARD_END()