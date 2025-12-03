// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/IndexedTable.h"
#include "RedpointEOSRooms/Database/ExclusivityKeySetFuncs.h"
#include "RedpointEOSRooms/Database/RoomDatabaseExclusivityState.h"
#include "RedpointEOSRooms/Database/RoomDatabaseTransaction.h"
#include "RedpointEOSRooms/Database/RoomScopeAndRoomUserId.h"
#include "RedpointEOSRooms/Providers/RoomProvider.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"
#include "RedpointEOSRooms/Requests/FeatureRequestsOnRequest.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomScope.h"
#include "RedpointEOSRooms/RoomUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2020585069, Redpoint::EOS::Rooms::Database)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Database;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::API;

class FRoomDatabase : public TSharedFromThis<FRoomDatabase>
{
private:
    FIndexedTable RoomTable;
    TMap<int, TSharedRef<FRoomDatabaseTransaction>> TransactionMap;
    FIndexedTable ExclusiveKeyTable;
    int NextTransactionId;

    void ValidateDatabaseState() const;

public:
    FRoomDatabase();
    UE_NONCOPYABLE(FRoomDatabase);
    ~FRoomDatabase();

    typedef TDelegate<void()> FReleaseReservation;
    typedef TDelegate<void(int TransactionId, const FReleaseReservation &ReleaseReservation)> FExecuteWithReservation;
    typedef TDelegate<void(const FError &LockError)> FRejectWithError;
    typedef TDelegate<void()> FRoomLeft;
    typedef TDelegate<void(int TransactionId, const FRoomRef &Room, const FRoomLeft &RoomLeft)> FLeaveRoom;

private:
    static void ComputeRequiredExclusiveKeys(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FFeatureRequestsOnRequest *FeatureRequestsOnRequest,
        const FRoomPtr &Room,
        const FRoomSearchResultPtr &RoomSearchResult,
        const TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> &FeatureHandlers,
        TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> &OutRequiredExclusiveKeys);

    enum class EAcquisitionStatus : uint8
    {
        // All required exclusive keys are acquirable right now.
        Acquirable,

        // The transaction must be deferred since it's blocked by another transaction right now.
        Defer,

        // The transaction must be rejected because there is another room holding an exclusivity key and the feature key
        // behaviour is RejectNewRoom.
        Reject,
    };

    [[nodiscard]] EAcquisitionStatus CanExclusiveKeysBeAcquired(
        const TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> &RequiredExclusiveKeys,
        ERoomScope RoomScope,
        FRoomUserId LocalUserId,
        FRoomPtr Room,
        int TransactionId,
        TSet<TSharedRef<FRoomDatabaseExclusivityState>> &OutExclusivityStatesToAcquire,
        TSet<TSharedRef<FRoomDatabaseExclusivityState>> &OutExclusivityStatesToReserve,
        TArray<FRoomRef> &OutRoomsToLeave,
        TSet<int> &OutBlockedByTransactionIds) const;

    void ExecuteTransactionNow(
        const TSharedRef<FRoomDatabaseTransaction> &StartingTransaction,
        const FRoomPtr &Room,
        const TSet<TSharedRef<FRoomDatabaseExclusivityState>> &ExclusivityStatesToAcquire,
        const TSet<TSharedRef<FRoomDatabaseExclusivityState>> &ExclusivityStatesToReserve,
        const TArray<FRoomRef> &RoomsToLeaveArray,
        const FExecuteWithReservation &ExecuteWithReservation,
        const FRejectWithError &RejectWithError,
        const FLeaveRoom &LeaveRoom);

    void RemoveTransactionIdFromPendingTransactionsAndAttemptContinue(int RemovedTransactionId);

    void AttemptContinueOfDeferredTransaction(
        TSharedRef<FRoomDatabaseTransaction> ContinuingTransaction,
        TSet<TSharedRef<FRoomDatabaseExclusivityState>> OriginalExclusivityStatesToReserve,
        TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> RequiredExclusiveKeys,
        FRoomPtr Room,
        FExecuteWithReservation ExecuteWithReservation,
        FRejectWithError RejectWithError,
        FLeaveRoom LeaveRoom);

    void DeferTransaction(
        const TSharedRef<FRoomDatabaseTransaction> &StartingTransaction,
        const TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> &RequiredExclusiveKeys,
        const FRoomPtr &Room,
        const TSet<TSharedRef<FRoomDatabaseExclusivityState>> &ExclusivityStatesToReserve,
        const TSet<int> &BlockedByTransactionIds,
        const FExecuteWithReservation &ExecuteWithReservation,
        const FRejectWithError &RejectWithError,
        const FLeaveRoom &LeaveRoom);

public:
    void AcquireExclusiveKeys(
        ERoomScope RoomScope,
        FRoomUserId LocalUserId,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FFeatureRequestsOnRequest &FeatureRequestsOnRequest,
        const FRoomPtr &Room,
        const FRoomSearchResultPtr &RoomSearchResult,
        const TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> &FeatureHandlers,
        const FExecuteWithReservation &ExecuteWithReservation,
        const FRejectWithError &RejectWithError,
        const FLeaveRoom &LeaveRoom);

    void TransactionalAddRoom(int TransactionId, const FRoomRef &Room);
    void TransactionalUpdateRoom(int TransactionId, const FRoomRef &Room);
    void TransactionalRemoveRoom(int TransactionId, const FRoomRef &Room);
    void NonTransactionalRemoveRoom(const FRoomRef &Room);

    FRoomScopeAndRoomUserId GetRoomScopeAndUserIdChecked(const FRoomRef &Room) const;
    TArray<FRoomRef> GetRooms(
        const FRoomScopeAndRoomUserId &RoomScopeAndRoomUserId,
        const FName &RoomNamespace,
        const TMap<FString, FRoomAttributeConstraint> &AttributeConstraints) const;
    TArray<TTuple<FRoomScopeAndRoomUserId, FRoomRef>> GetAllRooms() const;
    bool HasRoom(const FRoomRef &Room) const;

    enum class EEventStorageResult : uint8
    {
        Stored,
        NotStored,
    };
    EEventStorageResult StoreEventBroadcastAgainstConflictingTransactionIfPresent(
        ERoomScope RoomScope,
        FRoomUserId LocalUserId,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> &FeatureHandlers,
        const FSimpleDelegate &EventBroadcast);

    void Reset();
};

}

namespace Redpoint::EOS::Rooms::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(2020585069, Redpoint::EOS::Rooms::Database, FRoomDatabaseExclusivityState)
REDPOINT_EOS_FILE_NS_EXPORT(2020585069, Redpoint::EOS::Rooms::Database, FRoomDatabaseTransaction)
REDPOINT_EOS_FILE_NS_EXPORT(2020585069, Redpoint::EOS::Rooms::Database, FRoomDatabase)
}

REDPOINT_EOS_CODE_GUARD_END()