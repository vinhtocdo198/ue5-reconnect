// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Database/RoomDatabaseTransaction.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2834721303, Redpoint::EOS::Rooms::Database)
{

FRoomDatabaseTransaction::FRoomDatabaseTransaction(
    int InTransactionId,
    ERoomScope InRoomScope,
    FRoomUserId InLocalUserId)
    : TransactionId(InTransactionId)
    , RoomScope(InRoomScope)
    , LocalUserId(InLocalUserId)
    , bExecuting(false)
    , WaitingOnTransactionIds()
    , ExclusivityStates()
    , DeferredEventBroadcasts()
{
}

void FRoomDatabaseTransaction::StartExecution(
    const TSet<TSharedRef<FRoomDatabaseExclusivityState>> &InExclusivityStates)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    checkf(!this->bExecuting, TEXT("This transaction has already started!"));
    this->bExecuting = true;
    this->ExclusivityStates = InExclusivityStates;
}

}

REDPOINT_EOS_CODE_GUARD_END()