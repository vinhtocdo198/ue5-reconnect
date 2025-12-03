// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Database/RoomDatabase.h"

#include "CoreGlobals.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2020585069, Redpoint::EOS::Rooms::Database)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Rooms::Database;

typedef TTuple<ERoomScope, FRoomUserId, FString> FRoomScopeAndRoomUserIdAndString;

static auto CRoomsRoomScopeAndRoomUserId = FIndexedColumnDefinition<FRoomScopeAndRoomUserId>(
    FName(TEXT("RoomScopeAndRoomUserId")),
    [](const FRoomScopeAndRoomUserId &A, const FRoomScopeAndRoomUserId &B) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

        if (A.Get<0>() != B.Get<0>())
        {
            return false;
        }
        if (A.Get<0>() == ERoomScope::Global)
        {
            return true;
        }
        else
        {
            return A.Get<1>() == B.Get<1>();
        }
    },
    [](const FRoomScopeAndRoomUserId &K) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

        if (K.Get<0>() == ERoomScope::Global)
        {
            return GetTypeHash(ERoomScope::Global);
        }
        else
        {
            return HashCombine(GetTypeHash(ERoomScope::Global), GetTypeHash(K.Get<1>()));
        }
    });
static auto CRoomsRoomRef = FIndexedColumnDefinition<FRoomRef>(
    FName(TEXT("RoomRef")),
    [](const FRoomRef &A, const FRoomRef &B) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

        return &A.Get() == &B.Get();
    },
    [](const FRoomRef &K) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

        return GetTypeHash(&K.Get());
    });

static auto CExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey =
    FIndexedColumnDefinition<FRoomScopeAndRoomUserIdAndString>(
        FName(TEXT("RoomScopeAndRoomUserIdAndExclusiveKey")),
        [](const FRoomScopeAndRoomUserIdAndString &A, const FRoomScopeAndRoomUserIdAndString &B) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

            if (A.Get<0>() != B.Get<0>())
            {
                return false;
            }
            if (A.Get<0>() == ERoomScope::Global)
            {
                return A.Get<2>() == B.Get<2>();
            }
            else
            {
                return A.Get<1>() == B.Get<1>() && A.Get<2>() == B.Get<2>();
            }
        },
        [](const FRoomScopeAndRoomUserIdAndString &K) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

            if (K.Get<0>() == ERoomScope::Global)
            {
                return HashCombine(GetTypeHash(ERoomScope::Global), GetTypeHash(K.Get<2>()));
            }
            else
            {
                return HashCombine(
                    GetTypeHash(ERoomScope::Global),
                    HashCombine(GetTypeHash(K.Get<1>()), GetTypeHash(K.Get<2>())));
            }
        });
static auto CExclusiveKeyExclusivityState = FIndexedColumnDefinition<TSharedRef<FRoomDatabaseExclusivityState>>(
    FName(TEXT("ExclusivityState")),
    [](const TSharedRef<FRoomDatabaseExclusivityState> &A, const TSharedRef<FRoomDatabaseExclusivityState> &B) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

        return &A.Get() == &B.Get();
    },
    [](const TSharedRef<FRoomDatabaseExclusivityState> &K) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

        return GetTypeHash(&K.Get());
    });

void FRoomDatabase::ValidateDatabaseState() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    using namespace ::Redpoint::EOS::Core::Id;

#if !UE_BUILD_SHIPPING
    if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
    {
        UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("Room database state:"));
        UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("  Next transaction ID: %d"), this->NextTransactionId);
        UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("  Room table:"));
        for (const auto &RoomRow : this->RoomTable.GetAllRows())
        {
            auto Room = CRoomsRoomRef.Get(RoomRow);
            UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("    %s:"), *Room->GetRoomId()->ToString());
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Bound to user ID: %s"),
                *GetUserIdString(Room->GetBoundForLocalUser()));
        }
        UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("  Exclusive key table:"));
        for (const auto &ExclusiveRow : this->ExclusiveKeyTable.GetAllRows())
        {
            auto Info = CExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey.Get(ExclusiveRow);
            auto State = CExclusiveKeyExclusivityState.Get(ExclusiveRow);
            UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("    %s:"), *State->ExclusiveKey);
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Room scope: %s"),
                Info.Get<0>() == ERoomScope::Global ? TEXT("Global") : TEXT("User"));
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Bound to user ID: %s"),
                Info.Get<0>() == ERoomScope::Global ? TEXT("-") : (*GetUserIdString(Info.Get<1>())));
            UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("      Exclusive key: %s"), *Info.Get<2>());
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Reserved for transaction ID: %d"),
                State->ReservedForTransactionId);
            {
                TArray<FString> PendingReservationForTransactionIdsStr;
                for (const auto &PendingTransactionId : State->PendingReservationForTransactionIds)
                {
                    PendingReservationForTransactionIdsStr.Add(FString::Printf(TEXT("%d"), PendingTransactionId));
                }
                UE_LOG(
                    LogRedpointEOSRooms,
                    VeryVerbose,
                    TEXT("      Pending reservation for transaction IDs: %s"),
                    *FString::Join(PendingReservationForTransactionIdsStr, TEXT(", ")));
            }
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Reserved for leave room: %s"),
                State->ReservedForLeaveRoom ? TEXT("Yes") : TEXT("No"));
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Assign at transaction end: %s"),
                State->AssignAtTransactionEnd ? TEXT("Yes") : TEXT("No"));
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Reserved for room ID: %s"),
                State->Room.IsValid() ? (*State->Room->GetRoomId()->ToString()) : TEXT("-"));
        }
        UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("  Transaction map:"));
        for (const auto &TransactionKV : this->TransactionMap)
        {
            UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("    %d:"), TransactionKV.Key);
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Transaction ID: %d"),
                TransactionKV.Value->TransactionId);
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Room scope: %s"),
                TransactionKV.Value->RoomScope == ERoomScope::Global ? TEXT("Global") : TEXT("User"));
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Bound to user ID: %s"),
                TransactionKV.Value->RoomScope == ERoomScope::Global
                    ? TEXT("Global")
                    : (*GetUserIdString(TransactionKV.Value->LocalUserId)));
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Executing: %s"),
                TransactionKV.Value->bExecuting ? TEXT("Yes") : TEXT("No"));
            {
                TArray<FString> WaitingOnTransactionIdsStr;
                for (const auto &WaitingTransactionId : TransactionKV.Value->WaitingOnTransactionIds)
                {
                    WaitingOnTransactionIdsStr.Add(FString::Printf(TEXT("%d"), WaitingTransactionId));
                }
                UE_LOG(
                    LogRedpointEOSRooms,
                    VeryVerbose,
                    TEXT("      Waiting on transaction IDs: %s"),
                    *FString::Join(WaitingOnTransactionIdsStr, TEXT(", ")));
            }
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Exclusive states: (%d %s)"),
                TransactionKV.Value->ExclusivityStates.Num(),
                TransactionKV.Value->ExclusivityStates.Num() == 1 ? TEXT("state") : TEXT("states"));
            for (const auto &ExclusiveState : TransactionKV.Value->ExclusivityStates)
            {
                UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("        %s"), *ExclusiveState->ExclusiveKey);
            }
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Deferred event broadcasts: (%d deferred events)"),
                TransactionKV.Value->DeferredEventBroadcasts.Num());
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("      Continue after deferral: %s"),
                TransactionKV.Value->ContinueAfterDeferral.IsBound() ? TEXT("Bound") : TEXT("Unbound"));
        }

        // @note: Only do the checks if the logging is also bumped up to VeryVerbose, so that we're not frequently
        // running these checks unless we need to diagnose an issue.
        TSet<int> TransactionIds;
        for (const auto &ExclusiveRow : this->ExclusiveKeyTable.GetAllRows())
        {
            auto State = CExclusiveKeyExclusivityState.Get(ExclusiveRow);
            if (State->Room.IsValid())
            {
                auto TableRow = this->RoomTable.FindFirstByValue(CRoomsRoomRef.Create(State->Room.ToSharedRef()));
                checkf(TableRow.IsValid(), TEXT("Room should exist if an exclusive key is associated with it."));
            }
            if (State->ReservedForTransactionId != 0)
            {
                checkf(
                    this->TransactionMap.Contains(State->ReservedForTransactionId),
                    TEXT("Transaction should exist if exclusive key is reserved for it."));
                TransactionIds.Add(State->ReservedForTransactionId);
            }
        }
#if DO_CHECK
        for (const auto &TransactionKV : this->TransactionMap)
        {
            for (const auto &ExclusiveState : TransactionKV.Value->ExclusivityStates)
            {
                checkf(
                    this->ExclusiveKeyTable.FindFirstByValue(CExclusiveKeyExclusivityState.Create(ExclusiveState))
                        .IsValid(),
                    TEXT("Expected exclusive state to be in exclusive key table."));
                checkf(
                    ExclusiveState->ReservedForTransactionId == TransactionKV.Key,
                    TEXT("Expected exclusive state's reserved transaction ID to match transaction ID."));
            }
        }
#endif
    }
#endif
}

FRoomDatabase::FRoomDatabase()
    : RoomTable()
    , TransactionMap()
    , ExclusiveKeyTable()
    , NextTransactionId(1000)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    this->RoomTable.AddIndexedColumn(CRoomsRoomScopeAndRoomUserId, false).AddIndexedColumn(CRoomsRoomRef, true);
    this->ExclusiveKeyTable.AddIndexedColumn(CExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey, true)
        .AddIndexedColumn(CExclusiveKeyExclusivityState, true);
}

FRoomDatabase::~FRoomDatabase()
{
}

void FRoomDatabase::ComputeRequiredExclusiveKeys(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FFeatureRequestsOnRequest *FeatureRequestsOnRequest,
    const FRoomPtr &Room,
    const FRoomSearchResultPtr &RoomSearchResult,
    const TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> &FeatureHandlers,
    TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> &RequiredExclusiveKeys)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    // Determine all the exclusive keys we need to get a hold of in order to run this operation.
    if (Room.IsValid())
    {
        for (const auto &RoomFeatureName : Room->GetRoomFeatureNames())
        {
            auto RoomFeatureBase = Room->GetRoomFeatureBase(RoomFeatureName);
            auto RoomFeatureHandlerBase = FeatureHandlers.FindRef(RoomFeatureName);
            if (RoomFeatureBase.IsValid() && RoomFeatureHandlerBase.IsValid() &&
                RoomFeatureHandlerBase->GetRoomExclusivityKeyFromFeature().IsBound())
            {
                RequiredExclusiveKeys.Add(RoomFeatureHandlerBase->GetRoomExclusivityKeyFromFeature().Execute(
                    RoomProvider,
                    Room.ToSharedRef(),
                    RoomFeatureBase.ToSharedRef()));
            }
        }
    }
    if (RoomSearchResult.IsValid())
    {
        for (const auto &RoomFeatureName : RoomSearchResult->GetRoomFeatureSearchStateNames())
        {
            auto RoomFeatureSearchStateBase = RoomSearchResult->GetRoomFeatureSearchStateBase(RoomFeatureName);
            auto RoomFeatureSearchStateHandlerBase = FeatureHandlers.FindRef(RoomFeatureName);
            if (RoomFeatureSearchStateBase.IsValid() && RoomFeatureSearchStateHandlerBase.IsValid() &&
                RoomFeatureSearchStateHandlerBase->GetRoomExclusivityKeyFromFeatureSearchState().IsBound())
            {
                RequiredExclusiveKeys.Add(
                    RoomFeatureSearchStateHandlerBase->GetRoomExclusivityKeyFromFeatureSearchState().Execute(
                        RoomProvider,
                        RoomFeatureSearchStateBase.ToSharedRef()));
            }
        }
    }
    if (FeatureRequestsOnRequest != nullptr)
    {
        for (const auto &FeatureRequestName : FeatureRequestsOnRequest->GetFeatureRequestNames())
        {
            auto FeatureRequestBase = FeatureRequestsOnRequest->GetFeatureRequestBase(FeatureRequestName);
            auto RoomFeatureHandlerBase = FeatureHandlers.FindRef(FeatureRequestName);
            if (FeatureRequestBase.IsValid() && RoomFeatureHandlerBase.IsValid() &&
                RoomFeatureHandlerBase->GetRoomExclusivityKeyFromFeatureRequest().IsBound())
            {
                RequiredExclusiveKeys.Add(RoomFeatureHandlerBase->GetRoomExclusivityKeyFromFeatureRequest()
                                              .Execute(RoomProvider, Room, FeatureRequestBase.ToSharedRef()));
            }
        }
    }

    // Remove the empty key from the set (rather than checking for it in the loops above).
    RequiredExclusiveKeys.Remove(FRoomProviderFeatureHandlerBase::FExclusivityKey(
        TEXT(""),
        FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::RejectNewRoom));
}

FRoomDatabase::EAcquisitionStatus FRoomDatabase::CanExclusiveKeysBeAcquired(
    const TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> &RequiredExclusiveKeys,
    ERoomScope RoomScope,
    FRoomUserId LocalUserId,
    FRoomPtr Room,
    int TransactionId,
    TSet<TSharedRef<FRoomDatabaseExclusivityState>> &ExclusivityStatesToAcquire,
    TSet<TSharedRef<FRoomDatabaseExclusivityState>> &ExclusivityStatesToReserve,
    TArray<FRoomRef> &RoomsToLeaveArray,
    TSet<int> &BlockedByTransactionIds) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    EAcquisitionStatus AcquisitionStatus = EAcquisitionStatus::Acquirable;

    // Create any rows in the exclusivity table for keys we aren't tracking yet.
    for (const auto &ExclusiveKey : RequiredExclusiveKeys)
    {
        auto ExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey =
            CExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey.Create(
                FRoomScopeAndRoomUserIdAndString(RoomScope, LocalUserId, ExclusiveKey.GetKey()));
        if (!this->ExclusiveKeyTable.FindFirstByValue(ExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey).IsValid())
        {
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("Transaction %d: Creating exclusive key in exclusive key table so that it can be reserved: %s"),
                TransactionId,
                *ExclusiveKey.GetKey());

            // @note: const_cast exception is because this operation is "safe" (i.e. won't change any existing
            // transaction state; just backfilling data so that we can complete evaluation).
            const_cast<FRoomDatabase *>(this)->ExclusiveKeyTable.AddRow(
                {ExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey,
                 CExclusiveKeyExclusivityState.Create(
                     MakeShared<FRoomDatabaseExclusivityState>(ExclusiveKey.GetKey()))});
        }
    }

    // Iterate through the exclusive keys and figure out what we need to do.
    for (const auto &ExclusiveKey : RequiredExclusiveKeys)
    {
        auto ExclusiveRow =
            this->ExclusiveKeyTable.FindFirstByValue(CExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey.Create(
                FRoomScopeAndRoomUserIdAndString(RoomScope, LocalUserId, ExclusiveKey.GetKey())));
        checkf(
            ExclusiveRow.IsValid(),
            TEXT("Expected exclusive key %s to be tracked in the exclusive key table (we should have just added it if "
                 "necessary!)"),
            *ExclusiveKey.GetKey());

        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Attempting to reserve exclusive key in exclusive key table: %s"),
            TransactionId,
            *ExclusiveKey.GetKey());

        auto State = CExclusiveKeyExclusivityState.Get(ExclusiveRow.ToSharedRef());
        if (State->IsReservedByEarlierTransaction(TransactionId))
        {
            // This exclusive key is already reserved by another transaction, so we must reject regardless of the
            // behaviour because we don't what state it is in.
            UE_LOG(
                LogRedpointEOSRooms,
                Warning,
                TEXT("Transaction %d: Unable to reserve exclusive key '%s' in exclusive key table because it is "
                     "already reserved for transaction %d."),
                TransactionId,
                *ExclusiveKey.GetKey(),
                State->ReservedForTransactionId);

            // This transaction is blocked.
            AcquisitionStatus = EAcquisitionStatus::Defer;
            BlockedByTransactionIds.Add(State->ReservedForTransactionId);

            // @note: We won't actually acquire this state because bBlocked will prevent us from continuing with
            // acquisition, but it is necessary so that we can also detect transitive exclusive keys from auto-leave
            // rooms.
            ExclusivityStatesToAcquire.Add(State);
        }
        else if (State->Room.IsValid() && State->Room == Room)
        {
            // The room we're processing has this exclusive key. We need to acquire it, but it's OK to acquire
            // because we already hold the right to do things with it.
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("Transaction %d: Reserving exclusive key '%s' in exclusive key table because the room we are "
                     "processing has this exclusive key."),
                TransactionId,
                *ExclusiveKey.GetKey());
            ExclusivityStatesToAcquire.Add(State);
        }
        else if (State->Room.IsValid())
        {
            // Another room has this exclusive key, but the exclusive key isn't actively changing.
            switch (ExclusiveKey.GetBehaviour())
            {
            case FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::RejectNewRoom:
                // We reject operations that would conflict, and don't automatically leave rooms.
                UE_LOG(
                    LogRedpointEOSRooms,
                    Error,
                    TEXT("Transaction %d: Not reserving exclusive key '%s' in exclusive key table because another room "
                         "has this exclusive key and the behaviour is 'RejectNewRoom'."),
                    TransactionId,
                    *ExclusiveKey.GetKey());
                if (AcquisitionStatus == EAcquisitionStatus::Acquirable)
                {
                    AcquisitionStatus = EAcquisitionStatus::Reject;
                }
                break;
            case FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour::ReplaceExistingRoom:
                // We need to leave the conflicting room automatically.
                UE_LOG(
                    LogRedpointEOSRooms,
                    VeryVerbose,
                    TEXT("Transaction %d: Marking exclusive key '%s' as a key to acquire later because another room "
                         "has this exclusive key and the behaviour is 'ReplaceExistingRoom'."),
                    TransactionId,
                    *ExclusiveKey.GetKey());
                ExclusivityStatesToAcquire.Add(State);
                break;
            default:
                checkf(false, TEXT("Unhandled FRoomProviderFeatureHandlerBase::EExclusivityKeyBehaviour value!"));
                break;
            }
        }
        else
        {
            // This exclusive key isn't being used at all.
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("Transaction %d: Reserving exclusive key '%s' in exclusive key table because it is not being used "
                     "at all."),
                TransactionId,
                *ExclusiveKey.GetKey());
            ExclusivityStatesToAcquire.Add(State);
        }
    }

    // Determine all of the rooms that we would need to leave first if this transaction proceeds.
    TSet<FRoomRef> RoomsToLeave;
    for (const auto &State : ExclusivityStatesToAcquire)
    {
        if (State->Room.IsValid() && State->Room != Room)
        {
            RoomsToLeave.Add(State->Room.ToSharedRef());
        }
    }
    RoomsToLeaveArray.Empty();
    for (const auto &RoomToLeave : RoomsToLeave)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Adding room '%s' to the auto-leave list."),
            TransactionId,
            *RoomToLeave->GetRoomId()->ToString());
        RoomsToLeaveArray.Add(RoomToLeave);
    }

    // The set of exclusivity states that we will reserve may be a superset of the exclusivity states we need to
    // acquire.
    ExclusivityStatesToReserve = ExclusivityStatesToAcquire;

    // For all of the rooms that we're going to auto-leave, add any exclusive keys they have that we aren't already
    // acquiring to the exclusive states to acquire list, so that when those rooms are left, we'll clean up all of the
    // exclusive keys they had (even if this transaction isn't interested in them).
    //
    // This is necessary because you can have a process like this:
    //
    // - Room A already exists and reserves exclusive keys 'X' and 'Y'.
    // - Attempt to create room B where the features only need exclusive key 'X' and the behaviour is auto-leave.
    // - The room B creation operation needs to be able to release exclusive key 'Y' after room A is left.
    // - Therefore the transaction needs to successfully acquire exclusive key 'Y' so that the transaction can proceed.
    //
    // Note that we can't just assume we'll acquire exclusive key 'Y', because the user could be running *another*
    // create transaction that is only directly interested in exclusive key 'Y'. Even though both create operations are
    // interested only in one exclusive key, because they will both cause an auto-leave of the same room, they can't run
    // concurrently.
    for (const auto &ExclusiveRow : this->ExclusiveKeyTable.GetAllRows())
    {
        auto State = CExclusiveKeyExclusivityState.Get(ExclusiveRow);
        if (State->Room.IsValid() && RoomsToLeave.Contains(State->Room.ToSharedRef()) &&
            !ExclusivityStatesToReserve.Contains(State))
        {
#if DO_CHECK
            {
                auto Info = CExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey.Get(ExclusiveRow);
                bool bSameScope = false;
                if (RoomScope == ERoomScope::Global)
                {
                    bSameScope = Info.Get<0>() == ERoomScope::Global;
                }
                else
                {
                    bSameScope = Info.Get<0>() == ERoomScope::User && Info.Get<1>() == LocalUserId;
                }
                checkf(
                    bSameScope,
                    TEXT("Expected bSameScope in room database exclusive state acquisition to be true. The only way "
                         "for rooms to be added to the 'RoomsToLeave' array is if the 'RoomScope' and 'LocalUserId' "
                         "already match (refer to iteration over 'RequiredExclusiveKeys' where it constructs the "
                         "exclusive key to lookup with the room scope and local user ID specified). Then, when "
                         "iterating through the exclusive key table, we only examine entries where 'State->Room' "
                         "exists in the 'RoomsToLeave'. Since 'RoomsToLeave' can only be populated from exclusive keys "
                         "where the 'RoomScope' and 'LocalUserId' match, we should only be able to find entries with a "
                         "matching 'State->Room' where 'RoomScope' and 'LocalUserId' also match.\n\nThe only scenario "
                         "where this check can fail is if the same 'FRoom' instance is assigned to multiple "
                         "'State->Room' fields where the room scope and local user ID of those exclusive key entries "
                         "differ, but since 'FRoom' instances are unique per scope (even when they refer to the same "
                         "lobby/session on the backend), the true bug of this check failing would be an 'FRoom' "
                         "instance being incorrectly assigned to an exclusive key state to which it does not belong "
                         "(where the room scope and/or local user ID differ between the exclusive key state and "
                         "room)."));
            }
#endif

            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("Transaction %d: Attempting to reserve exclusive key in exclusive key table as a transitive "
                     "dependency of auto-leave: %s"),
                TransactionId,
                *State->ExclusiveKey);

            if (State->IsReservedByEarlierTransaction(TransactionId))
            {
                // This exclusive key is already reserved by another transaction, so we must reject regardless of the
                // behaviour because we don't what state it is in.
                UE_LOG(
                    LogRedpointEOSRooms,
                    Warning,
                    TEXT("Transaction %d: Unable to reserve exclusive key '%s' in exclusive key table as a transitive "
                         "dependency of auto-leave because it is already reserved for transaction %d."),
                    TransactionId,
                    *State->ExclusiveKey,
                    State->ReservedForTransactionId);

                // This transaction is blocked.
                AcquisitionStatus = EAcquisitionStatus::Defer;
                BlockedByTransactionIds.Add(State->ReservedForTransactionId);

                // @note: We won't actually acquire this state because bBlocked will prevent us from continuing with
                // acquisition, but it makes the ExclusivityStatesToReserve array accurate for the states that would be
                // reserved if this transaction did continue.
                ExclusivityStatesToReserve.Add(State);
            }
            else
            {
                checkf(
                    !Room.IsValid() || !State->Room->GetRoomId()->IsEqual(Room->GetRoomId()),
                    TEXT("Did not expect target room to be on the auto-leave list!"));

                // We don't know what the original behaviour was for this exclusive key when it was first reserved, but
                // it doesn't matter. It's only being acquired because the act of auto-leaving on an exclusive key that
                // *IS* auto-leave will cause this transitive exclusive key to be released.
                UE_LOG(
                    LogRedpointEOSRooms,
                    VeryVerbose,
                    TEXT("Transaction %d: Marking exclusive key '%s' as a key to acquire later because it is a "
                         "transitive exclusive key acquired by a room that is about to under the auto-leave process."),
                    TransactionId,
                    *State->ExclusiveKey);
                ExclusivityStatesToReserve.Add(State);
            }
        }
    }
    return AcquisitionStatus;
}

void FRoomDatabase::ExecuteTransactionNow(
    const TSharedRef<FRoomDatabaseTransaction> &StartingTransaction,
    const FRoomPtr &Room,
    const TSet<TSharedRef<FRoomDatabaseExclusivityState>> &ExclusivityStatesToAcquire,
    const TSet<TSharedRef<FRoomDatabaseExclusivityState>> &ExclusivityStatesToReserve,
    const TArray<FRoomRef> &RoomsToLeaveArray,
    const FExecuteWithReservation &ExecuteWithReservation,
    const FRejectWithError &RejectWithError,
    const FLeaveRoom &LeaveRoom)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    StartingTransaction->StartExecution(ExclusivityStatesToReserve);

    // Go and reserve all of the exclusive keys for leaving.
    for (const auto &ExclusiveState : ExclusivityStatesToReserve)
    {
        // Reserve it for this transaction.
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Marking exclusive key '%s' as reserved by us."),
            StartingTransaction->TransactionId,
            *ExclusiveState->ExclusiveKey);
        ExclusiveState->ReservedForTransactionId = StartingTransaction->TransactionId;

        // If the state has a room present and it's not the room we're modifying, then the only way to reach this is if
        // we're auto-leaving this room.
        if (ExclusiveState->Room.IsValid() && ExclusiveState->Room != Room)
        {
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("Transaction %d: Marking exclusive key '%s' as 'reserved for leave room' because the room "
                     "currently using it is not the room we are processing."),
                StartingTransaction->TransactionId,
                *ExclusiveState->ExclusiveKey);
            ExclusiveState->ReservedForLeaveRoom = true;
        }

        // Set the assignment flag based on whether this exclusive key just needs to be reserved, or whether we're
        // actually acquiring it at the end of the transaction.
        ExclusiveState->AssignAtTransactionEnd = ExclusivityStatesToAcquire.Contains(ExclusiveState);
        if (ExclusiveState->AssignAtTransactionEnd)
        {
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("Transaction %d: Marking exclusive key '%s' as 'assign at transaction end' because this exclusive "
                     "key will be owned by the new room."),
                StartingTransaction->TransactionId,
                *ExclusiveState->ExclusiveKey);
        }
        else
        {
            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("Transaction %d: NOT marking exclusive key '%s' as 'assign at transaction end' because this "
                     "exclusive key is a transitive dependency that the new room does not acquire."),
                StartingTransaction->TransactionId,
                *ExclusiveState->ExclusiveKey);
        }
    }

    // Define the function to unwind transaction reservations.
    auto Unwind = [this, ExclusivityStatesToReserve, TransactionId = StartingTransaction->TransactionId]() {
        for (const auto &ExclusiveState : ExclusivityStatesToReserve)
        {
            checkf(
                ExclusiveState->ReservedForTransactionId == TransactionId,
                TEXT("Expected exclusive key '%s' to be held by transaction %d."),
                *ExclusiveState->ExclusiveKey,
                TransactionId);
            ExclusiveState->ReservedForTransactionId = 0;
            ExclusiveState->ReservedForLeaveRoom = false;
            ExclusiveState->AssignAtTransactionEnd = false;
        }

        // Copy out the deferred events for broadcasting and remove the transaction.
        TArray<FSimpleDelegate> DeferredEventBroadcasts;
        auto *Transaction = this->TransactionMap.Find(TransactionId);
        if (Transaction != nullptr)
        {
            // @note: Intentional copy so we can destruct the transaction.
            DeferredEventBroadcasts = (*Transaction)->DeferredEventBroadcasts;

            // Empty and clear transaction (so it will no be present for conflict checks).
            (*Transaction)->DeferredEventBroadcasts.Empty();
            this->TransactionMap.Remove(TransactionId);
        }

        UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("Finished transaction with transaction ID %d."), TransactionId);

        this->ValidateDatabaseState();

        // Now fire any deferred events.
        for (const auto &EventBroadcast : DeferredEventBroadcasts)
        {
            EventBroadcast.ExecuteIfBound();
        }

        // Continue any pending transactions where this transaction was the last one they were waiting on.
        this->RemoveTransactionIdFromPendingTransactionsAndAttemptContinue(TransactionId);
    };

#if !UE_BUILD_SHIPPING && DO_CHECK
    // Validate exclusivity states against our expectations for AutoLeave.
    for (const auto &ExclusiveState : StartingTransaction->ExclusivityStates)
    {
        checkf(
            ExclusiveState->Room == Room || !ExclusiveState->Room.IsValid() || ExclusiveState->ReservedForLeaveRoom,
            TEXT("Unexpected exclusivity state prior to auto-leaving rooms!"));
    }
#endif

    // Leave all rooms that need leaving before proceeding.
    FMultiOperation<FRoomRef, bool>::RunSP(
        this,
        RoomsToLeaveArray,
        [LeaveRoom,
         TransactionId =
             StartingTransaction->TransactionId](FRoomRef RoomToLeave, TFunction<void(bool OutValue)> OnDone) {
            if (LeaveRoom.IsBound())
            {
                // Ask the caller to leave the room
                UE_LOG(
                    LogRedpointEOSRooms,
                    VeryVerbose,
                    TEXT("Transaction %d: Requesting leave of room '%s'..."),
                    TransactionId,
                    *RoomToLeave->GetRoomId()->ToString());
                LeaveRoom.Execute(TransactionId, RoomToLeave, FRoomLeft::CreateLambda([OnDone]() {
                                      OnDone(true);
                                  }));
                return true;
            }
            return false;
        },
        [this,
         ExclusivityStatesToReserve,
         Room,
         Unwind,
         ExecuteWithReservation,
         RejectWithError,
         TransactionId = StartingTransaction->TransactionId](TArray<bool> OutValues) {
            // Ensure that all of the exclusive keys we need no longer have rooms associated with them.
            for (const auto &ExclusivityState : ExclusivityStatesToReserve)
            {
                if (ExclusivityState->Room.IsValid() && ExclusivityState->Room != Room)
                {
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Error,
                        TEXT("Expected room for exclusive key '%s' to be left, but exclusive key was still reserved!"),
                        *ExclusivityState->ExclusiveKey);
                    Unwind();
                    RejectWithError.ExecuteIfBound(Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(
                            TEXT("Expected room for exclusive key '%s' to be left, but exclusive key was still "
                                 "reserved!"),
                            *ExclusivityState->ExclusiveKey)));
                    return;
                }
            }

            // Execute the operation.
            if (ExecuteWithReservation.IsBound())
            {
                this->ValidateDatabaseState();
                ExecuteWithReservation.Execute(TransactionId, FReleaseReservation::CreateLambda(Unwind));
            }
            else
            {
                Unwind();
                RejectWithError.ExecuteIfBound(Errors::UnexpectedError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("ExecuteWithReservation is no longer bound!")));
                return;
            }
        });
}

void FRoomDatabase::RemoveTransactionIdFromPendingTransactionsAndAttemptContinue(int RemovedTransactionId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    // Get all transaction IDs.
    TArray<int> TransactionIds;
    this->TransactionMap.GetKeys(TransactionIds);
    TransactionIds.StableSort();

    // Iterate through the transactions, removing the transaction ID and continuing deferred transactions.
    for (const auto &TransactionId : TransactionIds)
    {
        auto *TransactionPtr = this->TransactionMap.Find(TransactionId);
        if (TransactionPtr == nullptr)
        {
            continue;
        }
        auto &Transaction = *TransactionPtr;

        if (Transaction->bExecuting)
        {
            continue;
        }

        if (Transaction->WaitingOnTransactionIds.Contains(RemovedTransactionId))
        {
            Transaction->WaitingOnTransactionIds.Remove(RemovedTransactionId);

            if (Transaction->WaitingOnTransactionIds.IsEmpty())
            {
                // This transaction is no longer waiting on anything. Continue it.
                checkf(
                    Transaction->ContinueAfterDeferral.IsBound(),
                    TEXT("Expected ContinueAfterDeferral to always be bound when a pending transaction should be "
                         "continued."));
                Transaction->ContinueAfterDeferral.Execute();
            }
        }
    }
}

void FRoomDatabase::AttemptContinueOfDeferredTransaction(
    TSharedRef<FRoomDatabaseTransaction> ContinuingTransaction,
    TSet<TSharedRef<FRoomDatabaseExclusivityState>> OriginalExclusivityStatesToReserve,
    TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> RequiredExclusiveKeys,
    FRoomPtr Room,
    FExecuteWithReservation ExecuteWithReservation,
    FRejectWithError RejectWithError,
    FLeaveRoom LeaveRoom)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    UE_LOG(
        LogRedpointEOSRooms,
        VeryVerbose,
        TEXT("Attempting to continue deferred transaction with transaction ID %d."),
        ContinuingTransaction->TransactionId);

    // Clear the continue callback.
    ContinuingTransaction->ContinueAfterDeferral.Unbind();

    // Remove ourselves from the pending reservation list of any exclusivity states that we originally added ourselves
    // to. No other transaction will get an opportunity to reserve those transaction states, even if we have to defer
    // again, because after we remove ourselves here and then immediately add ourselves again when we go into the
    // deferral block.
    for (const auto &ExclusiveState : OriginalExclusivityStatesToReserve)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Updating exclusive key '%s' and removing us from pending reservation list."),
            ContinuingTransaction->TransactionId,
            *ExclusiveState->ExclusiveKey);
        ExclusiveState->PendingReservationForTransactionIds.Remove(ContinuingTransaction->TransactionId);
    }

    // Attempt to acquire keys necessary to immediately start this transaction.
    TSet<TSharedRef<FRoomDatabaseExclusivityState>> ExclusivityStatesToAcquire;
    TSet<TSharedRef<FRoomDatabaseExclusivityState>> ExclusivityStatesToReserve;
    TArray<FRoomRef> RoomsToLeaveArray;
    TSet<int> BlockedByTransactionIds;
    auto AcquisitionStatus = this->CanExclusiveKeysBeAcquired(
        RequiredExclusiveKeys,
        ContinuingTransaction->RoomScope,
        ContinuingTransaction->LocalUserId,
        Room,
        ContinuingTransaction->TransactionId,
        ExclusivityStatesToAcquire,
        ExclusivityStatesToReserve,
        RoomsToLeaveArray,
        BlockedByTransactionIds);

    // If this transaction must now be rejected, fail immediately.
    if (AcquisitionStatus == EAcquisitionStatus::Reject)
    {
        // Copy out the deferred events for broadcasting and remove the transaction.
        auto TransactionId = ContinuingTransaction->TransactionId;
        TArray<FSimpleDelegate> DeferredEventBroadcasts;
        auto *Transaction = this->TransactionMap.Find(TransactionId);
        if (Transaction != nullptr)
        {
            // @note: Intentional copy so we can destruct the transaction.
            DeferredEventBroadcasts = (*Transaction)->DeferredEventBroadcasts;

            // Empty and clear transaction (so it will no be present for conflict checks).
            (*Transaction)->DeferredEventBroadcasts.Empty();
            this->TransactionMap.Remove(TransactionId);
        }

        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction with transaction ID %d was rejected after deferral."),
            TransactionId);

        this->ValidateDatabaseState();

        // Now fire any deferred events.
        for (const auto &EventBroadcast : DeferredEventBroadcasts)
        {
            EventBroadcast.ExecuteIfBound();
        }

        // Continue any pending transactions where this transaction was the last one they were waiting on.
        this->RemoveTransactionIdFromPendingTransactionsAndAttemptContinue(TransactionId);

        // Notify that the transaction was rejected.
        RejectWithError.ExecuteIfBound(Errors::DuplicateNotAllowed(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("Another room already exists that uses one or more of the exclusive features that this room is "
                     "trying to acquire, and the behaviour of the feature is set to RejectNewRoom. The logs will "
                     "contain additional information about which room is causing the conflict."))));
        return;
    }

    // If we are able to execute the transaction now, start it.
    if (AcquisitionStatus == EAcquisitionStatus::Acquirable)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Executing transaction after it had been deferred."),
            ContinuingTransaction->TransactionId);
        this->ExecuteTransactionNow(
            ContinuingTransaction,
            Room,
            ExclusivityStatesToAcquire,
            ExclusivityStatesToReserve,
            RoomsToLeaveArray,
            ExecuteWithReservation,
            RejectWithError,
            LeaveRoom);
    }
    // Otherwise this transaction is still blocked on some other transaction (that we didn't know would block it
    // earlier). Mark it as deferred again, which will set up the transaction dependencies again.
    else
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Transaction is being deferred again as it is still blocked on another transaction."),
            ContinuingTransaction->TransactionId);
        this->DeferTransaction(
            ContinuingTransaction,
            RequiredExclusiveKeys,
            Room,
            ExclusivityStatesToReserve,
            BlockedByTransactionIds,
            ExecuteWithReservation,
            RejectWithError,
            LeaveRoom);
    }
}

void FRoomDatabase::DeferTransaction(
    const TSharedRef<FRoomDatabaseTransaction> &StartingTransaction,
    const TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> &RequiredExclusiveKeys,
    const FRoomPtr &Room,
    const TSet<TSharedRef<FRoomDatabaseExclusivityState>> &ExclusivityStatesToReserve,
    const TSet<int> &BlockedByTransactionIds,
    const FExecuteWithReservation &ExecuteWithReservation,
    const FRejectWithError &RejectWithError,
    const FLeaveRoom &LeaveRoom)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    // Set the list of transactions we're waiting on.
    StartingTransaction->WaitingOnTransactionIds = BlockedByTransactionIds;

    // Add ourselves to the pending reservation list for all exclusivity states we'd have to reserve.
    for (const auto &ExclusiveState : ExclusivityStatesToReserve)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Updating exclusive key '%s' to include us in pending reservation list."),
            StartingTransaction->TransactionId,
            *ExclusiveState->ExclusiveKey);
        ExclusiveState->PendingReservationForTransactionIds.Add(StartingTransaction->TransactionId);
    }

    // Set the continue callback.
    StartingTransaction->ContinueAfterDeferral = FSimpleDelegate::CreateSP(
        this,
        &FRoomDatabase::AttemptContinueOfDeferredTransaction,
        StartingTransaction,
        ExclusivityStatesToReserve,
        RequiredExclusiveKeys,
        Room,
        ExecuteWithReservation,
        RejectWithError,
        LeaveRoom);

    // Validate the database state.
    this->ValidateDatabaseState();
}

void FRoomDatabase::AcquireExclusiveKeys(
    ERoomScope RoomScope,
    FRoomUserId LocalUserId,
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FFeatureRequestsOnRequest &FeatureRequestsOnRequest,
    const FRoomPtr &Room,
    const FRoomSearchResultPtr &RoomSearchResult,
    const TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> &FeatureHandlers,
    const FExecuteWithReservation &ExecuteWithReservation,
    const FRejectWithError &RejectWithError,
    const FLeaveRoom &LeaveRoom)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    this->ValidateDatabaseState();

    // Allocate transaction ID.
    int TransactionId = this->NextTransactionId++;
    UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("Starting a new transaction with transaction ID %d."), TransactionId);

    // Compute what exclusive keys this transaction needs.
    TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> RequiredExclusiveKeys;
    FRoomDatabase::ComputeRequiredExclusiveKeys(
        RoomProvider,
        &FeatureRequestsOnRequest,
        Room,
        RoomSearchResult,
        FeatureHandlers,
        RequiredExclusiveKeys);
    {
        TArray<FString> RequiredExclusiveKeyNames;
        for (const auto &ExclusiveKey : RequiredExclusiveKeys)
        {
            RequiredExclusiveKeyNames.Add(ExclusiveKey.GetKey());
        }
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Requires the following exclusive keys: %s"),
            TransactionId,
            *FString::Join(RequiredExclusiveKeyNames, TEXT(", ")));
    }

    // Attempt to acquire keys necessary to immediately start this transaction.
    TSet<TSharedRef<FRoomDatabaseExclusivityState>> ExclusivityStatesToAcquire;
    TSet<TSharedRef<FRoomDatabaseExclusivityState>> ExclusivityStatesToReserve;
    TArray<FRoomRef> RoomsToLeaveArray;
    TSet<int> BlockedByTransactionIds;
    auto AcquisitionStatus = this->CanExclusiveKeysBeAcquired(
        RequiredExclusiveKeys,
        RoomScope,
        LocalUserId,
        Room,
        TransactionId,
        ExclusivityStatesToAcquire,
        ExclusivityStatesToReserve,
        RoomsToLeaveArray,
        BlockedByTransactionIds);

    // If this transaction must be rejected, fail immediately.
    if (AcquisitionStatus == EAcquisitionStatus::Reject)
    {
        RejectWithError.ExecuteIfBound(Errors::DuplicateNotAllowed(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("Another room already exists that uses one or more of the exclusive features that this room is "
                     "trying to acquire, and the behaviour of the feature is set to RejectNewRoom. The logs will "
                     "contain additional information about which room is causing the conflict."))));
        return;
    }

    // Create the transaction entry.
    auto NewTransaction = MakeShared<FRoomDatabaseTransaction>(TransactionId, RoomScope, LocalUserId);
    this->TransactionMap.Add(TransactionId, NewTransaction);
    UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("Transaction %d: Created entry in transaction map."), TransactionId);

    // If we are able to execute immediately, do it now.
    if (AcquisitionStatus == EAcquisitionStatus::Acquirable)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Executing transaction immediately."),
            TransactionId);
        this->ExecuteTransactionNow(
            NewTransaction,
            Room,
            ExclusivityStatesToAcquire,
            ExclusivityStatesToReserve,
            RoomsToLeaveArray,
            ExecuteWithReservation,
            RejectWithError,
            LeaveRoom);
    }
    // Otherwise, if we have to defer the transaction, record what we're waiting on and set the callback to invoke when
    // ready.
    else
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Transaction %d: Transaction is being deferred as it is blocked on another transaction."),
            TransactionId);
        this->DeferTransaction(
            NewTransaction,
            RequiredExclusiveKeys,
            Room,
            ExclusivityStatesToReserve,
            BlockedByTransactionIds,
            ExecuteWithReservation,
            RejectWithError,
            LeaveRoom);
    }
}

void FRoomDatabase::TransactionalAddRoom(int TransactionId, const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

#if WITH_EDITOR
    if (!this->TransactionMap.Contains(TransactionId))
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("Transaction ID %d was missing from transaction map when FRoomDatabase::TransactionalAddRoom. This "
                 "can happen if the room operation completes after play-in-editor is stopped."),
            TransactionId);
        return;
    }
#endif

    const auto &Transaction = this->TransactionMap.FindChecked(TransactionId);

    // Add the room.
    this->RoomTable.AddRow(
        {CRoomsRoomScopeAndRoomUserId.Create(FRoomScopeAndRoomUserId(Transaction->RoomScope, Transaction->LocalUserId)),
         CRoomsRoomRef.Create(Room)});

    // Reserve all of the keys for this room.
    for (const auto &ExclusiveState : Transaction->ExclusivityStates)
    {
        // If the exclusive state should be assigned at transaction end, assign it to the new room.
        if (ExclusiveState->AssignAtTransactionEnd)
        {
            checkf(
                !ExclusiveState->Room.IsValid(),
                TEXT("Expected exclusive state for key '%s' to not have a room assigned to it by the time "
                     "TransactionalAddRoom is called!"),
                *ExclusiveState->ExclusiveKey);
            ExclusiveState->Room = Room;
        }
    }

    this->ValidateDatabaseState();
}

void FRoomDatabase::TransactionalUpdateRoom(int TransactionId, const FRoomRef &Room)
{
    // @note: Nothing to do here.
}

void FRoomDatabase::TransactionalRemoveRoom(int TransactionId, const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

#if WITH_EDITOR
    if (!this->TransactionMap.Contains(TransactionId))
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("Transaction ID %d was missing from transaction map when FRoomDatabase::TransactionalAddRoom. This "
                 "can happen if the room operation completes after play-in-editor is stopped."),
            TransactionId);
        return;
    }
#endif

    const auto &Transaction = this->TransactionMap.FindChecked(TransactionId);

    // Remove the room.
    auto RoomRow = this->RoomTable.FindFirstByValue(CRoomsRoomRef.Create(Room));
    checkf(RoomRow.IsValid(), TEXT("Expected room to exist in RoomTable upon call to TransactionalRemoveRoom!"));
    this->RoomTable.RemoveRow(RoomRow);

    // Iterate through the exclusivity states for this transaction, and for any exclusive state assigned to the room
    // that was left, clear that room assignment.
    //
    // The exclusivity states in a transaction may not just be for this room, as a transaction may have multiple
    // auto-leaves queued up with each of those auto-leave rooms having a subset of different exclusive keys. The new
    // room may be assigning all or a subset of the exclusivity states when it is finally added, but whether the
    // exclusivity state will be assigned to the new room has no bearing on clearing it for the old one, since the old
    // room is the one being left.
    //
    // Since the original transaction will have reserved even transitive exclusive keys, we don't need to worry about
    // the room having exclusivity keys that aren't in this list - all exclusivity keys of all auto-leave rooms are
    // reserved by the transaction (they just aren't marked as "assign at transaction end" if the new room doesn't want
    // them).
    //
    // @note: TransactionalRemoveRoom gets called as *each* room marked for auto-leave is left, and so we can't make any
    // assertions about the exclusive keys that didn't belong to this room. They may belong to other rooms undergoing
    // auto-leave, or they may be new exclusive keys that only the new room is interested in (i.e. the key may not have
    // a room assigned to it at all).
    for (const auto &ExclusiveState : Transaction->ExclusivityStates)
    {
        if (ExclusiveState->Room == Room)
        {
            // This exclusive key was assigned to the room that we left, and should now be cleared.
            ExclusiveState->Room = nullptr;
        }
    }

    this->ValidateDatabaseState();
}

void FRoomDatabase::NonTransactionalRemoveRoom(const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    auto Row = this->RoomTable.FindFirstByValue(CRoomsRoomRef.Create(Room));
    if (Row.IsValid())
    {
        auto ExclusiveRows = this->ExclusiveKeyTable.FindByPredicate([Room](const FIndexedRowConstRef &IndexedRow) {
            return CExclusiveKeyExclusivityState.Get(IndexedRow)->Room == Room;
        });
        for (const auto &ExclusiveRow : ExclusiveRows)
        {
            auto ExclusiveState = CExclusiveKeyExclusivityState.Get(ExclusiveRow);

            checkf(
                ExclusiveState->ReservedForTransactionId == 0 && !ExclusiveState->ReservedForLeaveRoom &&
                    !ExclusiveState->AssignAtTransactionEnd,
                TEXT("The exclusive state is reserved for a transaction (or has transaction flags that should be "
                     "cleared), but NonTransactionalRemoveRoom is only called from inside the "
                     "BroadcastOrDeferRoomEvent callback, which should only be firing event callbacks when there are "
                     "no transactions reserving exclusive keys for the room."));

            // Clear the assigned room.
            ExclusiveState->Room = nullptr;
        }
        this->RoomTable.RemoveRow(Row);
    }

    this->ValidateDatabaseState();
}

FRoomScopeAndRoomUserId FRoomDatabase::GetRoomScopeAndUserIdChecked(const FRoomRef &Room) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    auto Row = this->RoomTable.FindFirstByValue(CRoomsRoomRef.Create(Room));
    checkf(Row.IsValid(), TEXT("Room passed to GetRoomScopeAndUserIdChecked must be tracked!"));
    return CRoomsRoomScopeAndRoomUserId.Get(Row.ToSharedRef());
}

TArray<FRoomRef> FRoomDatabase::GetRooms(
    const FRoomScopeAndRoomUserId &RoomScopeAndRoomUserId,
    const FName &RoomNamespace,
    const TMap<FString, FRoomAttributeConstraint> &AttributeConstraints) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    TArray<FRoomRef> Results;
    for (const auto &Row : this->RoomTable.FindByValue(CRoomsRoomScopeAndRoomUserId.Create(RoomScopeAndRoomUserId)))
    {
        auto Room = CRoomsRoomRef.Get(Row);
        const auto &RoomAttributes = Room->GetRoomAttributes();

        if (!RoomNamespace.IsEqual(NAME_All) && !RoomNamespace.IsEqual(Room->GetRoomNamespace()))
        {
            continue;
        }

        bool bInclude = true;
        if (AttributeConstraints.Num() > 0)
        {
            for (const auto &KV : AttributeConstraints)
            {
                if (!bInclude)
                {
                    break;
                }
                switch (KV.Value.ConstraintType())
                {
                case FRoomAttributeConstraint::EConstraintType::Equal:
                    if (!RoomAttributes.Contains(KV.Key) || !(RoomAttributes[KV.Key] == KV.Value.Attribute()))
                    {
                        bInclude = false;
                    }
                    break;
                case FRoomAttributeConstraint::EConstraintType::NotEqual:
                    if (RoomAttributes.Contains(KV.Key) && !(RoomAttributes[KV.Key] != KV.Value.Attribute()))
                    {
                        bInclude = false;
                    }
                    break;
                case FRoomAttributeConstraint::EConstraintType::GreaterThan:
                    if (!RoomAttributes.Contains(KV.Key) || !(RoomAttributes[KV.Key] > KV.Value.Attribute()))
                    {
                        bInclude = false;
                    }
                    break;
                case FRoomAttributeConstraint::EConstraintType::GreaterThanOrEqual:
                    if (!RoomAttributes.Contains(KV.Key) || !(RoomAttributes[KV.Key] >= KV.Value.Attribute()))
                    {
                        bInclude = false;
                    }
                    break;
                case FRoomAttributeConstraint::EConstraintType::LessThan:
                    if (!RoomAttributes.Contains(KV.Key) || !(RoomAttributes[KV.Key] < KV.Value.Attribute()))
                    {
                        bInclude = false;
                    }
                    break;
                case FRoomAttributeConstraint::EConstraintType::LessThanOrEqual:
                    if (!RoomAttributes.Contains(KV.Key) || !(RoomAttributes[KV.Key] <= KV.Value.Attribute()))
                    {
                        bInclude = false;
                    }
                    break;
                case FRoomAttributeConstraint::EConstraintType::Distance:
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("EConstraintType::Distance constraint is being ignored on GetCurrentRooms operation."));
                    break;
                case FRoomAttributeConstraint::EConstraintType::AnyOf:
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("EConstraintType::AnyOf constraint is being ignored on GetCurrentRooms operation."));
                    break;
                case FRoomAttributeConstraint::EConstraintType::NotAnyOf:
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("EConstraintType::NotAnyOf constraint is being ignored on GetCurrentRooms operation."));
                    break;
                case FRoomAttributeConstraint::EConstraintType::OneOf:
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("EConstraintType::OneOf constraint is being ignored on GetCurrentRooms operation."));
                    break;
                case FRoomAttributeConstraint::EConstraintType::NotOneOf:
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("EConstraintType::NotOneOf constraint is being ignored on GetCurrentRooms operation."));
                    break;
                case FRoomAttributeConstraint::EConstraintType::Contains:
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("EConstraintType::Contains constraint is being ignored on GetCurrentRooms operation."));
                    break;
                default:
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("Unknown constraint is being ignored on GetCurrentRooms operation."));
                    break;
                }
            }
        }

        if (bInclude)
        {
            Results.Add(Room);
        }
    }
    return Results;
}

TArray<TTuple<FRoomScopeAndRoomUserId, FRoomRef>> FRoomDatabase::GetAllRooms() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    TArray<TTuple<FRoomScopeAndRoomUserId, FRoomRef>> Results;
    for (const auto &Row : this->RoomTable.GetAllRows())
    {
        Results.Add(
            TTuple<FRoomScopeAndRoomUserId, FRoomRef>(CRoomsRoomScopeAndRoomUserId.Get(Row), CRoomsRoomRef.Get(Row)));
    }
    return Results;
}

bool FRoomDatabase::HasRoom(const FRoomRef &Room) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    return this->RoomTable.FindFirstByValue(CRoomsRoomRef.Create(Room)).IsValid();
}

FRoomDatabase::EEventStorageResult FRoomDatabase::StoreEventBroadcastAgainstConflictingTransactionIfPresent(
    ERoomScope RoomScope,
    FRoomUserId LocalUserId,
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomRef &Room,
    const TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> &FeatureHandlers,
    const FSimpleDelegate &EventBroadcast)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    // Compute what exclusive keys this room uses.
    TSet<FRoomProviderFeatureHandlerBase::FExclusivityKey, FExclusivityKeySetFuncs> RequiredExclusiveKeys;
    FRoomDatabase::ComputeRequiredExclusiveKeys(
        RoomProvider,
        nullptr,
        Room,
        nullptr,
        FeatureHandlers,
        RequiredExclusiveKeys);
    TArray<FString> RequiredExclusiveKeyNames;
    {
        for (const auto &ExclusiveKey : RequiredExclusiveKeys)
        {
            RequiredExclusiveKeyNames.Add(ExclusiveKey.GetKey());
        }
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Event broadcast: Requires the following exclusive keys: %s"),
            *FString::Join(RequiredExclusiveKeyNames, TEXT(", ")));
    }

    // Go through all the exclusive keys and see if there are any transactions that are using them right now.
    TSet<int32> TransactionConflicts;
    for (const auto &ExclusiveKey : RequiredExclusiveKeys)
    {
        auto ExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey =
            CExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey.Create(
                FRoomScopeAndRoomUserIdAndString(RoomScope, LocalUserId, ExclusiveKey.GetKey()));
        auto Row = this->ExclusiveKeyTable.FindFirstByValue(ExclusiveKeyRoomScopeAndRoomUserIdAndExclusiveKey);
        if (Row.IsValid())
        {
            auto State = CExclusiveKeyExclusivityState.Get(Row.ToSharedRef());
            if (State->ReservedForTransactionId != 0)
            {
                TransactionConflicts.Add(State->ReservedForTransactionId);
            }
        }
    }

    // Try to pick the first transaction that exists and store the event against it.
    //
    // @note: I don't think we ever need to do anything more clever than this (I can't see a scenario where
    // TransactionConflicts *actually* has more than one entry).
    for (const auto &TransactionId : TransactionConflicts)
    {
        auto *Transaction = this->TransactionMap.Find(TransactionId);
        if (Transaction == nullptr)
        {
            // @note: This shouldn't actually be possible.
            continue;
        }

        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Event broadcast: Storing event as deferred event against transaction ID %d."),
            TransactionId);
        (*Transaction)->DeferredEventBroadcasts.Add(EventBroadcast);
        return EEventStorageResult::Stored;
    }

    // If we get to here, then there wasn't an existing transaction conflict to store against.
    UE_LOG(
        LogRedpointEOSRooms,
        VeryVerbose,
        TEXT("Event broadcast: There are no transactions currently using any of the exclusive keys: %s"),
        *FString::Join(RequiredExclusiveKeyNames, TEXT(", ")));
    return EEventStorageResult::NotStored;
}

void FRoomDatabase::Reset()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Database);

    this->RoomTable.Reset();
    this->TransactionMap.Reset();
    this->ExclusiveKeyTable.Reset();
    this->NextTransactionId = INT_MIN;
}

}

REDPOINT_EOS_CODE_GUARD_END()