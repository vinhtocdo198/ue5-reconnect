// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomSystemImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSAPI/UI/AcknowledgeEventId.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureRequest.h"
#include "RedpointEOSRooms/Filters/RoomFilterRegistry.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProvider.h"
#include "RedpointEOSRooms/Providers/RoomInviteImpl.h"
// #include "RedpointEOSRooms/Providers/Session/SessionRoomProvider.h"
#include "RedpointEOSRooms/RoomRequestedJoinImpl.h"
#include "RedpointEOSRooms/RoomRequestedLeaveImpl.h"
#include "RedpointEOSRooms/Synchronisation/RoomAttributeSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomFeatureSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomMaxMembersSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomMemberAttributeSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomMemberInfoSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomMemberSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomOwnerSynchronisationDifference.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(293683484, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Filters;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;
using namespace ::Redpoint::EOS::API::UI;

typedef TTuple<FRoomIdRef, FRoomUserId, FRoomUserId> FRoomIdRefAndLocalUserIdAndTargetUserId;

static auto CAnticipatedKick = FIndexedColumnDefinition<FRoomIdRefAndLocalUserIdAndTargetUserId>(
    FName(TEXT("AnticipatedKick")),
    [](const FRoomIdRefAndLocalUserIdAndTargetUserId &A, const FRoomIdRefAndLocalUserIdAndTargetUserId &B) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return A.Get<0>()->ToString() == B.Get<0>()->ToString() && A.Get<1>() == B.Get<1>() && A.Get<2>() == B.Get<2>();
    },
    [](const FRoomIdRefAndLocalUserIdAndTargetUserId &K) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return HashCombine(
            GetTypeHash(K.Get<0>()->ToString()),
            HashCombine(GetTypeHash(K.Get<1>()), GetTypeHash(K.Get<2>())));
    });

FRoomEventContext FRoomSystemImpl::GetRoomEventContextForRoom(const FRoomRef &Room) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    auto RoomScopeAndRoomUserId = this->RoomDatabase->GetRoomScopeAndUserIdChecked(Room);
    if (RoomScopeAndRoomUserId.Get<0>() == ERoomScope::Global)
    {
        return FRoomEventContext::Create();
    }
    else
    {
        return FRoomEventContext::Create(RoomScopeAndRoomUserId.Get<1>());
    }
}

// @note: This should not be used to defer events that genuinely need to happen immediately (or where we expect the
// transaction to have been released). Basically, only call this in the synchronisation code where room changes might be
// seen prior to the EOS SDK firing the callback for the operation we're running.
void FRoomSystemImpl::BroadcastOrDeferRoomEvent(
    const FRoomEventContext &Context,
    const FRoomRef &Room,
    const FSimpleDelegate &EventBroadcast)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Room->GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Warning,
            TEXT("Event broadcast: Can't find a room provider with name '%s'; this event can not be deferred if there "
                 "are conflicting transactions."),
            *Room->GetRoomId()->GetProviderName().ToString());
        EventBroadcast.ExecuteIfBound();
        return;
    }

    if (this->RoomDatabase->StoreEventBroadcastAgainstConflictingTransactionIfPresent(
            Context.RoomScope,
            Context.LocalUserId,
            RoomProvider.ToSharedRef(),
            Room,
            RoomProvider->GetFeatureHandlers(),
            EventBroadcast) == FRoomDatabase::EEventStorageResult::NotStored)
    {
        EventBroadcast.ExecuteIfBound();
        return;
    }
}

FRoomSystemImpl::FRoomSystemImpl(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , OnRoomJoinedDelegate()
    , OnRoomLeftDelegate()
    , OnRoomUpdatedDelegate()
    , OnRoomInviteReceivedDelegate()
    , OnRoomInviteRemovedDelegate()
    , OnRoomJoinRequestedDelegate()
    , OnRoomLeaveRequestedDelegate()
    , SynchronisationEvents()
    , SynchronisationDifferences()
    , SynchronisationUserEvictedFromRoom()
    , RoomDatabase(MakeShared<FRoomDatabase>())
    , UserRoomInvites()
    , RoomProviders()
    , AnticipatedKickTable()
    , NextSyntheticInviteId(1000)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    this->RoomProviders.Add(
        Providers::Lobby::LobbyRoomProviderName,
        Redpoint::EOS::Core::Utils::MakeSharedWithEvents<Providers::Lobby::FLobbyRoomProvider>(
            this,
            this->PlatformHandle));
    this->AnticipatedKickTable.AddIndexedColumn(CAnticipatedKick, true);
}

void FRoomSystemImpl::RegisterEvents()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // We need to bind the room provider's "room X from SDK event" to our own functions.
    for (const auto &RoomProvider : this->RoomProviders)
    {
        // When the room provider notifies us that the user wants to join a room directly, we have to forward
        // this to a JoinRoom operation on the room system.
        //
        // e.g. EOS_Sessions_AddNotifyJoinSessionAccepted
        RoomProvider.Value->OnRoomJoinRequestedFromSDKEvent() =
            FRoomProvider::FOnRoomJoinRequestedFromSDKEvent::CreateSP(
                this,
                &FRoomSystemImpl::OnRoomJoinRequestedFromSDKEvent);

        // When the room provider notifies us that the user wants to leave a room directly, we have to forward
        // this to a LeaveRoom operation on the room system.
        //
        // e.g. EOS_Sessions_AddNotifyLeaveSessionRequested
        RoomProvider.Value->OnRoomLeaveRequestedFromSDKEvent() =
            FRoomProvider::FOnRoomLeaveRequestedFromSDKEvent::CreateSP(
                this,
                &FRoomSystemImpl::OnRoomLeaveRequestedFromSDKEvent);

        // When the room provider notifies us of a new room invite, we need to add it to the receiving user's invite
        // list and propagate the event.
        //
        // e.g. EOS_Sessions_AddNotifySessionInviteReceived
        RoomProvider.Value->OnRoomInviteLocallyReceivedFromSDKEvent() =
            FRoomProvider::FOnRoomInviteLocallyReceivedFromSDKEvent::CreateSP(
                this,
                &FRoomSystemImpl::OnRoomInviteLocallyReceivedFromSDKEvent);

        // When the room provider notifies us that the user has accepted the invite via some external UI, forward the
        // notification out to the game to request it make a JoinRoom call for the invite.
        //
        // e.g. EOS_Sessions_AddNotifySessionInviteAccepted
        RoomProvider.Value->OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent() =
            FRoomProvider::FOnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent::CreateSP(
                this,
                &FRoomSystemImpl::OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent);

        // When the room provider notifies us that the local user has rejected a room invite via the overlay, we need to
        // remove it from the invite list and notify users of the room system. We do not need to call RejectInvite, as
        // this has already been done by the EOS SDK.
        //
        // e.g. EOS_Sessions_AddNotifySessionInviteRejected
        RoomProvider.Value->OnRoomInviteLocallyFullyRejectedFromSDKEvent() =
            FRoomProvider::FOnRoomInviteLocallyFullyRejectedFromSDKEvent::CreateSP(
                this,
                &FRoomSystemImpl::OnRoomInviteLocallyFullyRejectedFromSDKEvent);
    }

    // When the platform shuts down, we have to clear out our state to prevent us trying to act on rooms that no longer
    // exist and to release memory.
    this->PlatformHandle->OnShutdown().AddSP(this, &FRoomSystemImpl::OnPlatformShutdown);
}

void FRoomSystemImpl::OnPlatformShutdown(FPlatformInstance &)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    this->OnRoomJoinedDelegate.Reset();
    this->OnRoomLeftDelegate.Reset();
    this->OnRoomUpdatedDelegate.Reset();
    this->OnRoomInviteReceivedDelegate.Reset();
    this->OnRoomInviteRemovedDelegate.Reset();
    this->OnRoomJoinRequestedDelegate.Reset();
    this->OnRoomLeaveRequestedDelegate.Reset();
    this->SynchronisationEvents.Reset();
    this->SynchronisationDifferences.Reset();
    this->ExplicitlyLeavingRooms.Reset();
    this->SynchronisationUserEvictedFromRoom.Reset();
    this->RoomDatabase->Reset();
    this->UserRoomInvites.Reset();
    for (const auto &RoomProvider : this->RoomProviders)
    {
        RoomProvider.Value->Reset();
    }
    this->RoomProviders.Reset();
    this->AnticipatedKickTable.Reset();
    this->NextSyntheticInviteId = INT32_MIN;
}

void FRoomSystemImpl::OnRoomJoinRequestedFromSDKEvent(FRoomUserId ActorUserId, FRoomSearchResultRef RoomSearchResult)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    // We need to propagate this out of the room system so that external systems can handle it. This is because
    // at this point, we don't know the room scope or room features to attach.
    this->OnRoomJoinRequestedDelegate.Broadcast(
        RoomSearchResult->GetRoomNamespace(),
        FRoomEventContext::Create(ActorUserId),
        MakeShared<FRoomRequestedJoinImpl>(ActorUserId, RoomSearchResult));
}

void FRoomSystemImpl::OnRoomLeaveRequestedFromSDKEvent(FRoomUserId ActorUserId, FRoomIdRef RoomId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    // We need to propagate this out of the room system so that external systems can handle it. This is because the game
    // may want to prevent the leave room operation for proceeding.
    FRoomPtr Room;
    for (const auto &CandidateRoom : this->RoomDatabase->GetAllRooms())
    {
        if (!CandidateRoom.Get<1>()->GetRoomId()->IsEqual(RoomId))
        {
            continue;
        }
        if (CandidateRoom.Get<0>().Key == ERoomScope::Global ||
            (CandidateRoom.Get<0>().Key == ERoomScope::User && CandidateRoom.Get<0>().Value == ActorUserId))
        {
            Room = CandidateRoom.Get<1>();
            break;
        }
    }
    if (Room.IsValid())
    {
        this->OnRoomLeaveRequestedDelegate.Broadcast(
            Room.IsValid() ? Room->GetRoomNamespace() : NAME_All,
            FRoomEventContext::Create(ActorUserId),
            MakeShared<FRoomRequestedLeaveImpl>(ActorUserId, RoomId, Room.ToSharedRef()));
    }
    else
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("Received OnRoomLeaveRequestedFromSDKEvent for room ID '%s' but this room is not currently joined."),
            *RoomId->ToString());
    }
}

void FRoomSystemImpl::OnRoomInviteLocallyReceivedFromSDKEvent(
    FRoomInviteId RoomInviteId,
    FRoomUserId RoomInviteSenderUserId,
    FRoomUserId RoomInviteRecipientUserId,
    FRoomSearchResultRef RoomSearchResult)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    using namespace ::Redpoint::EOS::Core::Id;

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    // Create the room invite.
    auto RoomInvite =
        MakeShared<FRoomInviteImpl>(RoomInviteId, RoomInviteSenderUserId, RoomInviteRecipientUserId, RoomSearchResult);

    // Add the invite to the room invites list.
    auto &SpecificUserRoomInvites = this->UserRoomInvites.FindOrAdd(RoomInvite->GetRoomInviteRecipientUserId());
    if (SpecificUserRoomInvites.Contains(RoomInvite->GetRoomInviteId()))
    {
        // This invite is already tracked. This can happen if RestoreRoomInvites is called multiple times.
        return;
    }
    SpecificUserRoomInvites.Add(RoomInvite->GetRoomInviteId(), RoomInvite);

    // Log that we received an invite.
    UE_LOG(
        LogRedpointEOSRooms,
        Verbose,
        TEXT("Room invite for room ID '%s' for user ID '%s' was received."),
        *RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->ToString(),
        *GetUserIdString(RoomInviteRecipientUserId));

    // Broadcast the event to notify users of the room system.
    this->OnRoomInviteReceived(RoomInvite->GetRoomInviteSearchResult()->GetRoomNamespace())
        .Broadcast(FRoomEventContext::Create(RoomInvite->GetRoomInviteRecipientUserId()), RoomInvite);
}

void FRoomSystemImpl::OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent(
    FRoomInviteId RoomInviteId,
    FRoomUserId RoomInviteRecipientUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    // Lookup the invite in the room invites list.
    auto &SpecificUserRoomInvites = this->UserRoomInvites.FindOrAdd(RoomInviteRecipientUserId);
    auto RoomInvitePtr = SpecificUserRoomInvites.Find(RoomInviteId);
    if (RoomInvitePtr == nullptr)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("User accepted a room invite from the Epic Games overlay, but the invite is not known to the room "
                 "system. This request will be ignored."));
        return;
    }

    // We need to propagate this out of the room system so that external systems can handle it. This is because
    // at this point, we don't know the room scope or room features to attach.
    this->OnRoomJoinRequestedDelegate.Broadcast(
        (*RoomInvitePtr)->GetRoomInviteSearchResult()->GetRoomNamespace(),
        FRoomEventContext::Create(RoomInviteRecipientUserId),
        MakeShared<FRoomRequestedJoinImpl>(*RoomInvitePtr));
}

void FRoomSystemImpl::OnRoomInviteLocallyFullyRejectedFromSDKEvent(
    FRoomInviteId RoomInviteId,
    FRoomUserId RoomInviteRecipientUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    // Lookup the invite in the room invites list.
    auto &SpecificUserRoomInvites = this->UserRoomInvites.FindOrAdd(RoomInviteRecipientUserId);
    auto RoomInvitePtr = SpecificUserRoomInvites.Find(RoomInviteId);
    if (RoomInvitePtr == nullptr)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("User rejected a room invite from the Epic Games overlay, but the invite is not known to the room "
                 "system. This request will be ignored."));
        return;
    }
    auto RoomInvite = *RoomInvitePtr;

    // Select the room provider based on the invite.
    FName RoomProviderName = RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->GetProviderName();
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(RoomProviderName);
    if (!RoomProvider.IsValid())
    {
        // We can't reject this room invite because no such provider exists.
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("User rejected a room invite from the Epic Games overlay, but no such room provider '%s' exists in "
                 "the room system. This request will be ignored."),
            *RoomProviderName.ToString());
        return;
    }

    // Remove the invite from the invites list if it is still present.
    auto *SpecificUserRoomInvitesForRemoveCall = this->UserRoomInvites.Find(RoomInvite->GetRoomInviteRecipientUserId());
    if (SpecificUserRoomInvitesForRemoveCall != nullptr &&
        SpecificUserRoomInvitesForRemoveCall->Contains(RoomInvite->GetRoomInviteId()))
    {
        // Remove the invite.
        SpecificUserRoomInvitesForRemoveCall->Remove(RoomInvite->GetRoomInviteId());

        // Broadcast that the room invite has been removed.
        this->OnRoomInviteRemoved(RoomInvite->GetRoomInviteSearchResult()->GetRoomNamespace())
            .Broadcast(
                FRoomEventContext::Create(RoomInvite->GetRoomInviteRecipientUserId()),
                RoomInvite,
                IRoomSystem::ERoomInviteRemovedReason::Rejected);

        // Log that the room invite has been rejected.
        UE_LOG(
            LogRedpointEOSRooms,
            Verbose,
            TEXT("Room invite for room ID '%s' has been rejected via the overlay."),
            *RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->ToString());
    }
}

void FRoomSystemImpl::AutoLeaveRoom(int TransactionId, const FRoomRef &Room, const FRoomDatabase::FRoomLeft &RoomLeft)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        RoomLeft.ExecuteIfBound();
        return;
    }

    // Make sure the room instance is still known to us.
    if (!this->RoomDatabase->HasRoom(Room))
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("%s"),
            *Errors::InvalidParameters(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("The provided room is not known to the room system; have you already left it?"))
                 .ToLogString());
        RoomLeft.ExecuteIfBound();
        return;
    }

    // Select the room provider based on the room passed in.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Room->GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        // We can't update this room because no such provider exists.
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("%s"),
            *Errors::InvalidParameters(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The requested room provider does not exist."))
                 .ToLogString());
        RoomLeft.ExecuteIfBound();
        return;
    }

    // Indicate in ExplicitlyLeavingRooms that we're explicitly leaving this room through a ::LeaveRoom
    // call. This prevents synchronisation from double-firing OnRoomLeft.
    this->ExplicitlyLeavingRooms.Add(Room);

    // Initiate the asynchronous LeaveRoom operation through the room provider.
    auto RoomContext = this->GetRoomEventContextForRoom(Room);
    RoomProvider->LeaveRoom(
        Room,
        FLeaveRoomRequest(),
        FOnLeaveRoomComplete::CreateSPLambda(
            this,
            [this, Room, RoomLeft, TransactionId, RoomContext](FError ErrorCode) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                if (ErrorCode.WasSuccessful())
                {
                    // Prevent LeaveRoom from firing OnRoomLeft twice if two LeaveRoom calls are racing for
                    // the same room with the callback arriving on different frames.
                    if (this->RoomDatabase->HasRoom(Room))
                    {
                        this->RoomDatabase->TransactionalRemoveRoom(TransactionId, Room);

                        // Raise the OnRoomLeft event.
                        UE_LOG(
                            LogRedpointEOSRooms,
                            VeryVerbose,
                            TEXT("Raising OnRoomLeft event for event context '%s' and room ID '%s'."),
                            *RoomContext.ToString(),
                            *Room->GetRoomId()->ToString());

                        // @note: I don't believe this can be deferred, since systems listening for left/join events
                        // might need to remove the existing presence room before they set/add the new one.
                        this->OnRoomLeft(Room->GetRoomNamespace())
                            .Broadcast(RoomContext, Room, ERoomLeaveReason::LeftExplicitly);
                    }
                }

                // Remove the room from ExplicitlyLeavingRooms since we're now done.
                this->ExplicitlyLeavingRooms.Remove(Room);

                // We're done.
                RoomLeft.ExecuteIfBound();
            }));
}

TArray<FRoomRef> FRoomSystemImpl::GetCurrentRooms(const FGetCurrentRoomsRequest &Request)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return TArray<FRoomRef>();
    }

    return this->RoomDatabase->GetRooms(
        FRoomScopeAndRoomUserId(Request.RoomScope, Request.LocalUserId),
        Request.RoomNamespace,
        Request.AttributeConstraints);
}

void FRoomSystemImpl::CreateRoom(const FCreateRoomRequest &Request, const FOnCreateRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")),
            nullptr);
        return;
    }

    // Select the room provider based on the request.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Request.RoomProvider);
    if (!RoomProvider.IsValid())
    {
        // We can't create this room because no such provider exists.
        OnComplete.ExecuteIfBound(
            Errors::InvalidParameters(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The requested room provider does not exist.")),
            nullptr);
        return;
    }

    // Acquire exclusive keys for this operation.
    this->RoomDatabase->AcquireExclusiveKeys(
        Request.RoomScope,
        Request.LocalUser,
        RoomProvider.ToSharedRef(),
        Request,
        nullptr,
        nullptr,
        RoomProvider->GetFeatureHandlers(),
        FRoomDatabase::FExecuteWithReservation::CreateSPLambda(
            this,
            [this, RoomProvider, Request, OnComplete](
                int TransactionId,
                const FRoomDatabase::FReleaseReservation &ReleaseReservation) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // Initiate the asynchronous CreateRoom operation through the room provider.
                RoomProvider->CreateRoom(
                    Request,
                    FOnCreateRoomComplete::CreateSPLambda(
                        this,
                        [this, RoomProvider, Request, OnComplete, TransactionId, ReleaseReservation](
                            FError ErrorCode,
                            FRoomPtr Room) {
                            // On failure, early return.
                            if (!Room.IsValid())
                            {
                                ReleaseReservation.ExecuteIfBound();
                                OnComplete.ExecuteIfBound(ErrorCode, Room);
                                return;
                            }

                            // Add the room.
                            this->RoomDatabase->TransactionalAddRoom(TransactionId, Room.ToSharedRef());
                            ReleaseReservation.ExecuteIfBound();

                            // Raise the OnRoomJoined event.
                            auto EventContext = this->GetRoomEventContextForRoom(Room.ToSharedRef());
                            UE_LOG(
                                LogRedpointEOSRooms,
                                VeryVerbose,
                                TEXT("Raising OnRoomJoined event for event context '%s' and room ID '%s'."),
                                *EventContext.ToString(),
                                *Room->GetRoomId()->ToString());
                            this->OnRoomJoined(Room->GetRoomNamespace())
                                .Broadcast(EventContext, Room.ToSharedRef(), ERoomJoinReason::RequestedByCreateRoom);

                            // Return result.
                            OnComplete.ExecuteIfBound(ErrorCode, Room);
                        }));
            }),
        FRoomDatabase::FRejectWithError::CreateSPLambda(
            this,
            [OnComplete](const FError &LockError) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // We couldn't run the operation at all.
                OnComplete.ExecuteIfBound(LockError, nullptr);
            }),
        FRoomDatabase::FLeaveRoom::CreateSP(this, &FRoomSystemImpl::AutoLeaveRoom));
}

void FRoomSystemImpl::UpdateRoom(
    const FRoomRef &Room,
    const FUpdateRoomRequest &Request,
    const FOnUpdateRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")));
        return;
    }

    // Make sure the room instance is still known to us.
    if (!this->RoomDatabase->HasRoom(Room))
    {
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The provided room is not known to the room system; have you already left it?")));
        return;
    }

    // Select the room provider based on the room passed in.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Room->GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        // We can't update this room because no such provider exists.
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The requested room provider does not exist.")));
        return;
    }

    // Acquire exclusive keys for this operation.
    auto RoomScopeAndLocalUser = this->RoomDatabase->GetRoomScopeAndUserIdChecked(Room);
    this->RoomDatabase->AcquireExclusiveKeys(
        RoomScopeAndLocalUser.Key,
        RoomScopeAndLocalUser.Value,
        RoomProvider.ToSharedRef(),
        Request,
        Room,
        nullptr,
        RoomProvider->GetFeatureHandlers(),
        FRoomDatabase::FExecuteWithReservation::CreateSPLambda(
            this,
            [this, RoomProvider, Room, Request, OnComplete](
                int TransactionId,
                const FRoomDatabase::FReleaseReservation &ReleaseReservation) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // Initiate the asynchronous UpdateRoom operation through the room provider.
                RoomProvider->UpdateRoom(
                    Room,
                    Request,
                    FOnUpdateRoomComplete::CreateSPLambda(
                        this,
                        [this, Room, RoomProvider, OnComplete, ReleaseReservation, TransactionId](FError ErrorCode) {
                            this->RoomDatabase->TransactionalUpdateRoom(TransactionId, Room);
                            ReleaseReservation.ExecuteIfBound();

                            // We do synchronisation regardless of the status code, in case the SDK partially
                            // applying updates.
                            this->SynchroniseSingleRoomImmediately(RoomProvider.ToSharedRef(), Room);
                            OnComplete.ExecuteIfBound(ErrorCode);
                        }));
            }),
        FRoomDatabase::FRejectWithError::CreateSPLambda(
            this,
            [OnComplete](const FError &LockError) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // We couldn't run the operation at all.
                OnComplete.ExecuteIfBound(LockError);
            }),
        FRoomDatabase::FLeaveRoom::CreateSP(this, &FRoomSystemImpl::AutoLeaveRoom));
}

void FRoomSystemImpl::SearchRooms(const FSearchRoomsRequest &Request, const FOnSearchRoomsComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")),
            TArray<FRoomSearchResultRef>());
        return;
    }

    // Select the room provider based on the request.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Request.RoomProvider);
    if (!RoomProvider.IsValid())
    {
        // We can't create this room because no such provider exists.
        OnComplete.ExecuteIfBound(
            Errors::InvalidParameters(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The requested room provider does not exist.")),
            TArray<FRoomSearchResultRef>());
        return;
    }

    // Initiate the asynchronous SearchRooms operation through the room provider.
    RoomProvider->SearchRooms(Request, OnComplete);
}

void FRoomSystemImpl::JoinRoom(const FJoinRoomRequest &Request, const FOnJoinRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")),
            nullptr);
        return;
    }

    // Select the room provider based on the request.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Request.GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        // We can't join this room because no such provider exists.
        OnComplete.ExecuteIfBound(
            Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The requested room provider does not exist for the passed in room ID or room search result.")),
            nullptr);
        return;
    }

    // If we are joining by ID, we need to search for the room by ID so we can get the search result. We need the search
    // result so that we can acquire exclusive keys that can only be determined from the search result (i.e. features on
    // the host side that require joining clients to treat them exclusively).
    if (Request.RoomId.IsValid())
    {
        FSearchRoomsRequest SearchRequest(
            Request.LocalUser,
            Request.GetRoomId()->GetProviderName(),
            NAME_All,
            Request.RoomId.ToSharedRef());
        SearchRequest.bDurable = true;
        SearchRequest.MaxResultCount = 1;

        RoomProvider->SearchRooms(
            SearchRequest,
            IRoomSystem::FOnSearchRoomsComplete::CreateSPLambda(
                this,
                [this, Request, RoomProvider, OnComplete](
                    const FError &ErrorCode,
                    const TArray<FRoomSearchResultRef> &RoomSearchResults) {
                    REDPOINT_EOS_USE_LLM_TAG(Rooms);

                    if (!ErrorCode.WasSuccessful())
                    {
                        OnComplete.ExecuteIfBound(ErrorCode, nullptr);
                        return;
                    }

                    if (RoomSearchResults.Num() == 0)
                    {
                        OnComplete.ExecuteIfBound(
                            Errors::NotFound(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("Unable to find the target room in search results for join-by-ID operation.")),
                            nullptr);
                        return;
                    }

                    this->JoinRoomWithSearchResult(Request, RoomProvider, RoomSearchResults[0], OnComplete);
                }));
    }
    else if (Request.SearchResult.IsValid())
    {
        this->JoinRoomWithSearchResult(Request, RoomProvider, Request.SearchResult.ToSharedRef(), OnComplete);
    }
    else if (Request.RoomInvite.IsValid())
    {
        this->JoinRoomWithSearchResult(
            Request,
            RoomProvider,
            Request.RoomInvite->GetRoomInviteSearchResult(),
            OnComplete);
    }
    else if (Request.RoomRequestedJoin.IsValid())
    {
        this->JoinRoomWithSearchResult(
            Request,
            RoomProvider,
            Request.RoomRequestedJoin->GetRoomSearchResult(),
            OnComplete);
    }
    else
    {
        checkf(false, TEXT("Invalid FJoinRoomRequest to call FRoomSystemImpl::JoinRoom on!"));
    }
}

void FRoomSystemImpl::JoinRoomWithSearchResult(
    const FJoinRoomRequest &Request,
    const TSharedPtr<FRoomProvider> &RoomProvider,
    const FRoomSearchResultRef &SearchResult,
    const FOnJoinRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")),
            nullptr);
        return;
    }

    // Acquire exclusive keys for this operation.
    this->RoomDatabase->AcquireExclusiveKeys(
        Request.RoomScope,
        Request.LocalUser,
        RoomProvider.ToSharedRef(),
        Request,
        nullptr,
        SearchResult,
        RoomProvider->GetFeatureHandlers(),
        FRoomDatabase::FExecuteWithReservation::CreateSPLambda(
            this,
            [this, RoomProvider, Request, OnComplete](
                int TransactionId,
                const FRoomDatabase::FReleaseReservation &ReleaseReservation) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // Initiate the asynchronous Join operation through the room provider.
                RoomProvider->JoinRoom(
                    Request,
                    FOnJoinRoomComplete::CreateSPLambda(
                        this,
                        [this, Request, OnComplete, RoomProvider, TransactionId, ReleaseReservation](
                            FError ErrorCode,
                            FRoomPtr Room) {
                            REDPOINT_EOS_USE_LLM_TAG(Rooms);

                            // On failure, early return.
                            if (!Room.IsValid())
                            {
                                ReleaseReservation.ExecuteIfBound();
                                OnComplete.ExecuteIfBound(ErrorCode, Room);
                                return;
                            }

                            // Add the room.
                            this->RoomDatabase->TransactionalAddRoom(TransactionId, Room.ToSharedRef());
                            ReleaseReservation.ExecuteIfBound();

                            // If this request was based on an FRoomRequestedJoinRef, check if it has an invite
                            // associated with it and remove the invite if it did.
                            auto RoomInvite = Request.GetRoomInvite();
                            if (RoomInvite.IsValid())
                            {
                                // Lookup the invite in the room invites list.
                                auto *SpecificUserRoomInvites =
                                    this->UserRoomInvites.Find(RoomInvite->GetRoomInviteRecipientUserId());
                                if (SpecificUserRoomInvites != nullptr &&
                                    SpecificUserRoomInvites->Contains(RoomInvite->GetRoomInviteId()))
                                {
                                    // Remove the invite.
                                    SpecificUserRoomInvites->Remove(RoomInvite->GetRoomInviteId());

                                    // Broadcast that the room invite has been removed.
                                    this->OnRoomInviteRemoved(
                                            RoomInvite->GetRoomInviteSearchResult()->GetRoomNamespace())
                                        .Broadcast(
                                            FRoomEventContext::Create(RoomInvite->GetRoomInviteRecipientUserId()),
                                            RoomInvite.ToSharedRef(),
                                            IRoomSystem::ERoomInviteRemovedReason::Accepted);

                                    // Log that the room invite has been used with JoinRoom.
                                    UE_LOG(
                                        LogRedpointEOSRooms,
                                        Verbose,
                                        TEXT("Room invite for room ID '%s' has been used with JoinRoom by '%s'."),
                                        *RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->ToString(),
                                        *Redpoint::EOS::Core::Id::GetUserIdString(Request.LocalUser));
                                }
                            }

                            // Raise the OnRoomJoined event.
                            auto EventContext = this->GetRoomEventContextForRoom(Room.ToSharedRef());
                            UE_LOG(
                                LogRedpointEOSRooms,
                                VeryVerbose,
                                TEXT("Raising OnRoomJoined event for event context '%s' and room ID '%s'."),
                                *EventContext.ToString(),
                                *Room->GetRoomId()->ToString());
                            this->OnRoomJoined(Room->GetRoomNamespace())
                                .Broadcast(EventContext, Room.ToSharedRef(), ERoomJoinReason::RequestedByJoinRoom);

                            // Return result.
                            OnComplete.ExecuteIfBound(ErrorCode, Room);
                        }));
            }),
        FRoomDatabase::FRejectWithError::CreateSPLambda(
            this,
            [OnComplete](const FError &LockError) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // We couldn't run the operation at all.
                OnComplete.ExecuteIfBound(LockError, nullptr);
            }),
        FRoomDatabase::FLeaveRoom::CreateSP(this, &FRoomSystemImpl::AutoLeaveRoom));
}

void FRoomSystemImpl::LeaveRoom(
    const FRoomRef &Room,
    const FLeaveRoomRequest &RequestRef,
    const FOnLeaveRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")));
        return;
    }

    // Make sure the room instance is still known to us.
    if (!this->RoomDatabase->HasRoom(Room))
    {
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The provided room is not known to the room system; have you already left it?")));
        return;
    }

    // Run room filters on this request.
    auto Request = RequestRef;
    for (const auto &Filter : FRoomFilterRegistry::Get())
    {
        auto Action = Filter->OnLeaveRoom(this->PlatformHandle, Room, Request, OnComplete);
        if (Action == ERoomFilterAction::Terminate)
        {
            UE_LOG(
                LogRedpointEOSRooms,
                Verbose,
                TEXT(
                    "Room filter is intercepting 'LeaveRoom' request. The callback must be fired by the room filter."));
            return;
        }
    }

    // Select the room provider based on the room passed in.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Room->GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        // We can't update this room because no such provider exists.
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The requested room provider does not exist.")));
        return;
    }

    // Acquire exclusive keys for this operation.
    auto RoomScopeAndLocalUser = this->RoomDatabase->GetRoomScopeAndUserIdChecked(Room);
    auto RoomContext = this->GetRoomEventContextForRoom(Room);
    this->RoomDatabase->AcquireExclusiveKeys(
        RoomScopeAndLocalUser.Key,
        RoomScopeAndLocalUser.Value,
        RoomProvider.ToSharedRef(),
        FFeatureRequestsOnRequest(),
        Room,
        nullptr,
        RoomProvider->GetFeatureHandlers(),
        FRoomDatabase::FExecuteWithReservation::CreateSPLambda(
            this,
            [this, RoomProvider, Room, Request, OnComplete, RoomContext](
                int TransactionId,
                const FRoomDatabase::FReleaseReservation &ReleaseReservation) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // Indicate in ExplicitlyLeavingRooms that we're explicitly leaving this room through a ::LeaveRoom
                // call. This prevents synchronisation from double-firing OnRoomLeft.
                this->ExplicitlyLeavingRooms.Add(Room);

                // Initiate the asynchronous LeaveRoom operation through the room provider.
                RoomProvider->LeaveRoom(
                    Room,
                    Request,
                    FOnLeaveRoomComplete::CreateSPLambda(
                        this,
                        [this, Room, OnComplete, TransactionId, ReleaseReservation, RoomContext](FError ErrorCode) {
                            if (ErrorCode.WasSuccessful())
                            {
                                // Prevent LeaveRoom from firing OnRoomLeft twice if two LeaveRoom calls are racing for
                                // the same room with the callback arriving on different frames.
                                if (this->RoomDatabase->HasRoom(Room))
                                {
                                    this->RoomDatabase->TransactionalRemoveRoom(TransactionId, Room);
                                    ReleaseReservation.ExecuteIfBound();

                                    // Raise the OnRoomLeft event.
                                    UE_LOG(
                                        LogRedpointEOSRooms,
                                        VeryVerbose,
                                        TEXT("Raising OnRoomLeft event for event context '%s' and room ID '%s'."),
                                        *RoomContext.ToString(),
                                        *Room->GetRoomId()->ToString());
                                    this->OnRoomLeft(Room->GetRoomNamespace())
                                        .Broadcast(RoomContext, Room, ERoomLeaveReason::LeftExplicitly);
                                }
                                else
                                {
                                    ReleaseReservation.ExecuteIfBound();
                                }
                            }
                            else
                            {
                                ReleaseReservation.ExecuteIfBound();
                            }

                            // Remove the room from ExplicitlyLeavingRooms since we're now done.
                            this->ExplicitlyLeavingRooms.Remove(Room);

                            OnComplete.ExecuteIfBound(ErrorCode);
                        }));
            }),
        FRoomDatabase::FRejectWithError::CreateSPLambda(
            this,
            [OnComplete](const FError &LockError) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // We couldn't run the operation at all.
                OnComplete.ExecuteIfBound(LockError);
            }),
        FRoomDatabase::FLeaveRoom::CreateSP(this, &FRoomSystemImpl::AutoLeaveRoom));
}

void FRoomSystemImpl::PromoteRoomMember(
    const FRoomRef &Room,
    const FPromoteRoomMemberRequest &Request,
    const FOnPromoteRoomMemberComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")));
        return;
    }

    // Make sure the room instance is still known to us.
    if (!this->RoomDatabase->HasRoom(Room))
    {
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The provided room is not known to the room system; have you already left it?")));
        return;
    }

    // Select the room provider based on the room passed in.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Room->GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        // We can't update this room because no such provider exists.
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The requested room provider does not exist.")));
        return;
    }

    // Initiate the asynchronous PromoteRoomMember operation through the room provider.
    RoomProvider->PromoteRoomMember(
        Room,
        Request,
        FOnPromoteRoomMemberComplete::CreateSPLambda(this, [this, Room, RoomProvider, OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms);

            // We do synchronisation regardless of the status code, in case the SDK partially
            // applying updates.
            this->SynchroniseSingleRoomImmediately(RoomProvider.ToSharedRef(), Room);
            OnComplete.ExecuteIfBound(ErrorCode);
        }));
}

void FRoomSystemImpl::KickRoomMember(
    const FRoomRef &Room,
    const FKickRoomMemberRequest &Request,
    const FOnKickRoomMemberComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")));
        return;
    }

    // Make sure the room instance is still known to us.
    if (!this->RoomDatabase->HasRoom(Room))
    {
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The provided room is not known to the room system; have you already left it?")));
        return;
    }

    // Select the room provider based on the room passed in.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Room->GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        // We can't update this room because no such provider exists.
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The requested room provider does not exist.")));
        return;
    }

    // Initiate the asynchronous KickRoomMember operation through the room provider.
    RoomProvider->KickRoomMember(
        Room,
        Request,
        FOnKickRoomMemberComplete::CreateSPLambda(this, [this, Room, RoomProvider, OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms);

            // We do synchronisation regardless of the status code, in case the SDK partially
            // applying updates.
            this->SynchroniseSingleRoomImmediately(RoomProvider.ToSharedRef(), Room);
            OnComplete.ExecuteIfBound(ErrorCode);
        }));
}

IRoomSystem::FOnRoomJoined &FRoomSystemImpl::OnRoomJoined(const FName &InRoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->OnRoomJoinedDelegate[InRoomNamespace];
}

IRoomSystem::FOnRoomLeft &FRoomSystemImpl::OnRoomLeft(const FName &InRoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->OnRoomLeftDelegate[InRoomNamespace];
}

IRoomSystem::FOnRoomUpdated &FRoomSystemImpl::OnRoomUpdated(const FName &InRoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->OnRoomUpdatedDelegate[InRoomNamespace];
}

TArray<FRoomInviteRef> FRoomSystemImpl::GetCurrentRoomInvites(
    const FRoomUserId &InLocalUser,
    const FName &InRoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        return TArray<FRoomInviteRef>();
    }

    // Retrieve all the room invites for this user.
    TArray<FRoomInviteRef> RoomInvites;
    for (const auto &RoomInvite : this->UserRoomInvites.FindRef(InLocalUser))
    {
        if (InRoomNamespace.IsEqual(NAME_All) ||
            InRoomNamespace.IsEqual(RoomInvite.Value->GetRoomInviteSearchResult()->GetRoomNamespace()))
        {
            RoomInvites.Add(RoomInvite.Value);
        }
    }
    return RoomInvites;
}

void FRoomSystemImpl::RestoreRoomInvites(
    const FRestoreRoomInvitesRequest &Request,
    const FOnRestoreRoomInvitesComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")));
        return;
    }

    // Get the room providers into an array for FMultiOperation.
    TArray<TSharedRef<FRoomProvider>> RoomProvidersArray;
    for (const auto &KV : this->RoomProviders)
    {
        RoomProvidersArray.Add(KV.Value.ToSharedRef());
    }

    // Restore room invites across all of the room providers and return the aggregated error result.
    Redpoint::EOS::Core::Utils::FMultiOperation<TSharedRef<FRoomProvider>, TSharedPtr<FError>>::RunSP(
        this,
        RoomProvidersArray,
        [this,
         Request](TSharedPtr<FRoomProvider> RoomProvider, TFunction<void(TSharedPtr<FError> OutValue)> OnDone) -> bool {
            REDPOINT_EOS_USE_LLM_TAG(Rooms);

            RoomProvider->RestoreRoomInvites(
                Request,
                FOnRestoreRoomInvitesComplete::CreateSPLambda(this, [OnDone](FError ErrorCode) {
                    REDPOINT_EOS_USE_LLM_TAG(Rooms);

                    OnDone(MakeShared<FError>(ErrorCode));
                }));
            return true;
        },
        [OnComplete](TArray<TSharedPtr<FError>> OutValues) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms);

            for (const auto &Error : OutValues)
            {
                if (Error.IsValid() && !Error->WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(*Error);
                    return;
                }
            }
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            return;
        });
}

void FRoomSystemImpl::SendRoomInvite(
    const FRoomRef &Room,
    const FSendRoomInviteRequest &Request,
    const FOnSendRoomInviteComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")));
        return;
    }

    // Make sure the room instance is still known to us.
    if (!this->RoomDatabase->HasRoom(Room))
    {
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The provided room is not known to the room system; have you already left it?")));
        return;
    }

    // Select the room provider based on the room passed in.
    TSharedPtr<FRoomProvider> RoomProvider = this->RoomProviders.FindRef(Room->GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        // We can't update this room because no such provider exists.
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The requested room provider does not exist.")));
        return;
    }

    // Initiate the asynchronous SendRoomInvite operation through the room provider.
    RoomProvider->SendRoomInvite(
        Room,
        Request,
        FOnSendRoomInviteComplete::CreateSPLambda(this, [OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms);

            // @note: We don't run synchronisation here, because this operation has no effect on the room.
            OnComplete.ExecuteIfBound(ErrorCode);
        }));
}

void FRoomSystemImpl::RejectRoomInvite(const FRoomInviteRef &RoomInvite, const FOnRejectRoomInviteComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")));
        return;
    }

    // Make sure this invite is still known to the recipient user ID.
    auto *SpecificUserRoomInvites = this->UserRoomInvites.Find(RoomInvite->GetRoomInviteRecipientUserId());
    if (SpecificUserRoomInvites == nullptr || !SpecificUserRoomInvites->Contains(RoomInvite->GetRoomInviteId()))
    {
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The provided room invite is not known to the room system; was it already used or rejected?")));
        return;
    }

    // Select the room provider based on the room invite passed in.
    TSharedPtr<FRoomProvider> RoomProvider =
        this->RoomProviders.FindRef(RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->GetProviderName());
    if (!RoomProvider.IsValid())
    {
        // We can't update this room because no such provider exists.
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The requested room provider does not exist.")));
        return;
    }

    // Initiate the asynchronous RejectRoomInvite operation through the room provider.
    RoomProvider->RejectRoomInvite(
        RoomInvite,
        FOnRejectRoomInviteComplete::CreateSPLambda(this, [this, RoomInvite, OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms);

            // We remove the room invite regardless of the room provider result. If this operation failed, log the
            // cause.
            if (!ErrorCode.WasSuccessful())
            {
                if (!GIsAutomationTesting)
                {
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Error,
                        TEXT("User rejected a room invite from the Epic Games overlay, but the operation to reject the "
                             "invite failed. The room invite will still be removed from the invite list for the game, "
                             "but the Epic Online Services backend may not notify the sending player. The error result "
                             "was: %s"),
                        *ErrorCode.ToLogString());
                }
            }

            // Remove the invite from the invites list if it is still present.
            auto *SpecificUserRoomInvitesForRemoveCall =
                this->UserRoomInvites.Find(RoomInvite->GetRoomInviteRecipientUserId());
            if (SpecificUserRoomInvitesForRemoveCall != nullptr &&
                SpecificUserRoomInvitesForRemoveCall->Contains(RoomInvite->GetRoomInviteId()))
            {
                // Remove the invite.
                SpecificUserRoomInvitesForRemoveCall->Remove(RoomInvite->GetRoomInviteId());

                // Broadcast that the room invite has been removed.
                this->OnRoomInviteRemoved(RoomInvite->GetRoomInviteSearchResult()->GetRoomNamespace())
                    .Broadcast(
                        FRoomEventContext::Create(RoomInvite->GetRoomInviteRecipientUserId()),
                        RoomInvite,
                        IRoomSystem::ERoomInviteRemovedReason::Rejected);

                // Log that the room invite has been rejected.
                UE_LOG(
                    LogRedpointEOSRooms,
                    Verbose,
                    TEXT("Room invite for room ID '%s' has been rejected via the game."),
                    *RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->ToString());
            }

            // Notify the callback.
            OnComplete.ExecuteIfBound(ErrorCode);
        }));
}

IRoomSystem::FOnRoomInviteReceived &FRoomSystemImpl::OnRoomInviteReceived(const FName &InRoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->OnRoomInviteReceivedDelegate[InRoomNamespace];
}

IRoomSystem::FOnRoomInviteRemoved &FRoomSystemImpl::OnRoomInviteRemoved(const FName &InRoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->OnRoomInviteRemovedDelegate[InRoomNamespace];
}

IRoomSystem::FOnRoomJoinRequested &FRoomSystemImpl::OnRoomJoinRequested(const FName &InRoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->OnRoomJoinRequestedDelegate[InRoomNamespace];
}

IRoomSystem::FOnRoomLeaveRequested &FRoomSystemImpl::OnRoomLeaveRequested(const FName &InRoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->OnRoomLeaveRequestedDelegate[InRoomNamespace];
}

bool FRoomSystemImpl::HasRoomProvider(const FName &InProviderName)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomProviders.Contains(InProviderName);
}

void FRoomSystemImpl::RejectRoomRequestedJoin(
    const FRoomRequestedJoinRef &RoomRequestedJoin,
    const FOnRejectRoomRequestedJoinComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately fail.
    if (this->PlatformHandle->IsShutdown())
    {
        OnComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The platform handle has been shutdown.")));
        return;
    }

    auto SearchResult = RoomRequestedJoin->GetRoomSearchResult();
    if (SearchResult->GetAssociatedUiEventId().IsSet())
    {
        // @todo: Do we need some kind of flag to indicate the result code that we should pass back?
        OnComplete.ExecuteIfBound(ConvertError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Notifying Epic Games overlay of decline of requested room join."),
            FAcknowledgeEventId::Execute(
                this->PlatformHandle,
                FAcknowledgeEventId::Options{
                    SearchResult->GetAssociatedUiEventId().GetValue(),
                    EOS_EResult::EOS_InvalidRequest})));
    }
    else
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Verbose,
            TEXT("RejectRoomRequestedJoin operation is being ignored as the supplied RoomRequestedJoin does not have a "
                 "UI event (this is expected when requested room joins occur via the runtime platform and not the Epic "
                 "Games overlay)."));
        OnComplete.ExecuteIfBound(Errors::Success());
    }
}

void FRoomSystemImpl::ComputeSynchronisationDifferencesAndRaiseEvents(
    const FRoomRef &RoomRef,
    const TArray<FSynchronisationDifferenceRef> &Differences,
    const FRoomEventContext &EventContext)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    using namespace ::Redpoint::EOS::Core::Id;

    // If the platform handle has been shutdown, immediately return.
    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    // We need to aggregate differences to figure out what
    // events to fire. For example, we should only fire
    // one OnRoomUpdated event per local user regardless of how
    // many room attributes change.
    bool bRoomUpdated = false;
    bool bRoomLeft = false;
    FRoomUpdateInfo RoomUpdateInfo;
    ERoomLeaveReason RoomLeaveReason = ERoomLeaveReason::LeftExplicitly;
    TSet<FRoomUserId> MembersJoined;
    TMap<FRoomUserId, FRoomMemberUpdateInfo> MembersUpdated;
    TSet<FRoomUserId> MembersPromoted;
    TMap<FRoomUserId, ERoomLeaveReason> MembersLeft;
    for (const auto &Difference : Differences)
    {
        if (ISynchronisationDifference::Get<FRoomAttributeSynchronisationDifference>(Difference).IsValid() ||
            ISynchronisationDifference::Get<FRoomFeatureSynchronisationDifference>(Difference).IsValid() ||
            ISynchronisationDifference::Get<FRoomMaxMembersSynchronisationDifference>(Difference).IsValid())
        {
            // Generic room-wide update.
            bRoomUpdated = true;

            auto AttributeDifference =
                ISynchronisationDifference::Get<FRoomAttributeSynchronisationDifference>(Difference);
            auto FeatureDifference = ISynchronisationDifference::Get<FRoomFeatureSynchronisationDifference>(Difference);
            auto MaxMembersDifference =
                ISynchronisationDifference::Get<FRoomMaxMembersSynchronisationDifference>(Difference);
            if (AttributeDifference.IsValid())
            {
                switch (AttributeDifference->GetChangeType())
                {
                case FRoomAttributeSynchronisationDifference::EChangeType::Added:
                    RoomUpdateInfo.AttributesAdded.Add(AttributeDifference->GetAttributeKey());
                    break;
                case FRoomAttributeSynchronisationDifference::EChangeType::Modified:
                    RoomUpdateInfo.AttributesUpdated.Add(AttributeDifference->GetAttributeKey());
                    break;
                case FRoomAttributeSynchronisationDifference::EChangeType::Removed:
                    RoomUpdateInfo.AttributesDeleted.Add(AttributeDifference->GetAttributeKey());
                    break;
                default:
                    checkf(false, TEXT("Unhandled FRoomAttributeSynchronisationDifference::EChangeType attribute!"));
                    break;
                }
            }
            if (FeatureDifference.IsValid())
            {
                RoomUpdateInfo.FeaturesUpdated.Add(FeatureDifference->GetFeatureName());
            }
            if (MaxMembersDifference.IsValid())
            {
                RoomUpdateInfo.bMaxMembersChanged = true;
            }
        }
        else if (
            auto OwnerDifference = ISynchronisationDifference::Get<FRoomOwnerSynchronisationDifference>(Difference))
        {
            // Changing owners is an update on the room.
            bRoomUpdated = true;
            RoomUpdateInfo.bOwnerChanged = true;

            // For rooms with members, promoting a member is also
            // a member promote event.
            if (!OwnerDifference->GetNewOwner()->IsDedicatedServer())
            {
                MembersPromoted.Add(OwnerDifference->GetNewOwner()->GetUserId());
            }
        }
        else if (
            auto MemberDifference = ISynchronisationDifference::Get<FRoomMemberSynchronisationDifference>(Difference))
        {
            // Raises either a member join or member leave event.
            switch (MemberDifference->GetChangeType())
            {
            case FRoomMemberSynchronisationDifference::EChangeType::Joined:
                MembersJoined.Add(MemberDifference->GetMemberUserId());
                break;
            case FRoomMemberSynchronisationDifference::EChangeType::Left:
            case FRoomMemberSynchronisationDifference::EChangeType::Disconnected:
            case FRoomMemberSynchronisationDifference::EChangeType::Kicked:
            case FRoomMemberSynchronisationDifference::EChangeType::Closed: {
                ERoomLeaveReason RoomMemberLeaveReason = ERoomLeaveReason::LeftExplicitly;
                switch (MemberDifference->GetChangeType())
                {
                case FRoomMemberSynchronisationDifference::EChangeType::Left:
                    RoomMemberLeaveReason = ERoomLeaveReason::LeftExplicitly;
                    break;
                case FRoomMemberSynchronisationDifference::EChangeType::Disconnected:
                    RoomMemberLeaveReason = ERoomLeaveReason::Disconnected;
                    break;
                case FRoomMemberSynchronisationDifference::EChangeType::Kicked:
                    RoomMemberLeaveReason = ERoomLeaveReason::KickedFromRoom;
                    break;
                case FRoomMemberSynchronisationDifference::EChangeType::Closed:
                    RoomMemberLeaveReason = ERoomLeaveReason::RoomDestroyed;
                    break;
                case FRoomMemberSynchronisationDifference::EChangeType::Joined:
                default:
                    checkf(false, TEXT("Expected GetChangeType to be one of the supported leave types!"));
                    break;
                }
                MembersLeft.Add(MemberDifference->GetMemberUserId(), RoomMemberLeaveReason);
                if (MemberDifference->IsForBoundUser())
                {
                    bRoomLeft = true;
                    RoomLeaveReason = RoomMemberLeaveReason;
                }
                break;
            }
            default:
                checkf(false, TEXT("Expected GetChangeType to be one of the supported leave types!"));
                break;
            }
        }
        else if (
            auto MemberAttrDifference =
                ISynchronisationDifference::Get<FRoomMemberAttributeSynchronisationDifference>(Difference))
        {
            // Raises a member update event.
            auto &RoomMemberUpdateInfo = MembersUpdated.FindOrAdd(MemberAttrDifference->GetMemberUserId());
            switch (MemberAttrDifference->GetChangeType())
            {
            case FRoomMemberAttributeSynchronisationDifference::EChangeType::Added:
                RoomMemberUpdateInfo.AttributesAdded.Add(MemberAttrDifference->GetAttributeKey());
                break;
            case FRoomMemberAttributeSynchronisationDifference::EChangeType::Modified:
                RoomMemberUpdateInfo.AttributesUpdated.Add(MemberAttrDifference->GetAttributeKey());
                break;
            case FRoomMemberAttributeSynchronisationDifference::EChangeType::Removed:
                RoomMemberUpdateInfo.AttributesDeleted.Add(MemberAttrDifference->GetAttributeKey());
                break;
            default:
                checkf(false, TEXT("Unhandled FRoomMemberAttributeSynchronisationDifference::EChangeType attribute!"));
                break;
            }
        }
        else if (
            auto MemberInfoDifference =
                ISynchronisationDifference::Get<FRoomMemberInfoSynchronisationDifference>(Difference))
        {
            // Raises a member update event.
            auto &RoomMemberUpdateInfo = MembersUpdated.FindOrAdd(MemberAttrDifference->GetMemberUserId());
            RoomMemberUpdateInfo.bInfoUpdated = true;
        }
        else
        {
            checkf(
                false /* unhandled room difference */,
                TEXT("The difference type %s is not handled."),
                *Difference->GetDifferenceType().ToString());
        }
    }

    if (bRoomUpdated)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Raising OnRoomUpdated event for event context '%s' and room ID '%s'."),
            *EventContext.ToString(),
            *RoomRef->GetRoomId()->ToString());
        this->BroadcastOrDeferRoomEvent(
            EventContext,
            RoomRef,
            FSimpleDelegate::CreateSPLambda(this, [this, EventContext, RoomRef, RoomUpdateInfo]() {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                this->OnRoomUpdated(RoomRef->GetRoomNamespace()).Broadcast(EventContext, RoomRef, RoomUpdateInfo);
            }));
    }
    for (const auto &Member : MembersJoined)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Raising OnRoomMemberJoined event for event context '%s' and room ID '%s' and room member ID '%s'."),
            *EventContext.ToString(),
            *RoomRef->GetRoomId()->ToString(),
            *GetUserIdString(Member));
        this->BroadcastOrDeferRoomEvent(
            EventContext,
            RoomRef,
            FSimpleDelegate::CreateLambda([EventContext, RoomRef, Member]() {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                RoomRef->OnRoomMemberJoined().Broadcast(EventContext, RoomRef, Member);
            }));
    }
    for (const auto &MemberKV : MembersUpdated)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Raising OnRoomMemberUpdated event for event context '%s' and room ID '%s' and room member ID '%s'."),
            *EventContext.ToString(),
            *RoomRef->GetRoomId()->ToString(),
            *GetUserIdString(MemberKV.Key));
        this->BroadcastOrDeferRoomEvent(
            EventContext,
            RoomRef,
            FSimpleDelegate::CreateLambda([EventContext, RoomRef, MemberKV]() {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                RoomRef->OnRoomMemberUpdated().Broadcast(EventContext, RoomRef, MemberKV.Key, MemberKV.Value);
            }));
    }
    for (const auto &Member : MembersPromoted)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Raising OnRoomMemberPromoted event for event context '%s' and room ID '%s' and room member ID '%s'."),
            *EventContext.ToString(),
            *RoomRef->GetRoomId()->ToString(),
            *GetUserIdString(Member));
        this->BroadcastOrDeferRoomEvent(
            EventContext,
            RoomRef,
            FSimpleDelegate::CreateLambda([EventContext, RoomRef, Member]() {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                RoomRef->OnRoomMemberPromoted().Broadcast(EventContext, RoomRef, Member);
            }));
    }
    for (const auto &MemberKV : MembersLeft)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Raising OnRoomMemberLeft event for event context '%s' and room ID '%s' and room member ID '%s'."),
            *EventContext.ToString(),
            *RoomRef->GetRoomId()->ToString(),
            *GetUserIdString(MemberKV.Key));
        this->BroadcastOrDeferRoomEvent(
            EventContext,
            RoomRef,
            FSimpleDelegate::CreateLambda([EventContext, RoomRef, MemberKV]() {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                RoomRef->OnRoomMemberLeft().Broadcast(EventContext, RoomRef, MemberKV.Key, MemberKV.Value);
            }));
    }
    if (bRoomLeft && !this->ExplicitlyLeavingRooms.Contains(RoomRef))
    {
        // Notify room filters of an unexpected room leave.
        for (const auto &Filter : FRoomFilterRegistry::Get())
        {
            Filter->OnRoomLeftExternally(this->PlatformHandle, RoomRef, RoomLeaveReason);
        }

        // Now broadcast the OnRoomLeft event.
        UE_LOG(
            LogRedpointEOSRooms,
            VeryVerbose,
            TEXT("Raising OnRoomLeft event for event context '%s' and room ID '%s'."),
            *EventContext.ToString(),
            *RoomRef->GetRoomId()->ToString());
        this->BroadcastOrDeferRoomEvent(
            EventContext,
            RoomRef,
            FSimpleDelegate::CreateSPLambda(this, [this, EventContext, RoomRef, RoomLeaveReason]() {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // Remove this room from the rooms list, so that any user code that looks at GetCurrentRooms inside the
                // OnRoomLeft handler won't find it.
                this->RoomDatabase->NonTransactionalRemoveRoom(RoomRef);

                // Fire the event.
                this->OnRoomLeft(RoomRef->GetRoomNamespace()).Broadcast(EventContext, RoomRef, RoomLeaveReason);
            }));
    }
}

class FSingleRoomSynchronisationContext : public Synchronisation::ISynchronisationContext
{
public:
    FRoomIdRef RoomId;
    TArray<Synchronisation::FSynchronisationEventRef> SynchronisationEvents;
    TArray<Synchronisation::FSynchronisationDifferenceRef> SynchronisationDifferences;
    TRoomIdMap<TSet<FRoomUserId>> SynchronisationUserEvictedFromRoom;
    FIndexedTable AnticipatedKickTable;

    FSingleRoomSynchronisationContext(const FRoomIdRef &InRoomId, const FIndexedTable &InCurrentAnticipatedKickTable)
        : RoomId(InRoomId)
        , SynchronisationEvents()
        , SynchronisationDifferences()
        , SynchronisationUserEvictedFromRoom()
        , AnticipatedKickTable()
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        this->AnticipatedKickTable.AddIndexedColumn(CAnticipatedKick, true);
        for (const auto &Row : InCurrentAnticipatedKickTable.GetAllRows())
        {
            this->AnticipatedKickTable.AddRow({CAnticipatedKick.Create(CAnticipatedKick.Get(Row))});
        }
    }

    virtual void AddSynchronisationEvent(const FRoomIdRef &InRoomId, const FSynchronisationEventRef &InEvent) override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        if (!this->RoomId->IsEqual(InRoomId))
        {
            return;
        }
        this->SynchronisationEvents.Add(InEvent);
    }

    virtual TArray<FSynchronisationEventRef> GetSynchronisationEventOfTypeBase(
        const FRoomIdRef &InRoomId,
        FName InEventType) const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        if (!this->RoomId->IsEqual(InRoomId))
        {
            return TArray<FSynchronisationEventRef>();
        }
        TArray<FSynchronisationEventRef> Results;
        for (const auto &Event : this->SynchronisationEvents)
        {
            if (Event->GetEventType().IsEqual(InEventType))
            {
                Results.Add(Event);
            }
        }
        return Results;
    }

    virtual void RecordMemberEvictedFromRoom(const FRoomIdRef &InRoomId, const FRoomUserId &InRoomMemberId) override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        auto &EvictionList = this->SynchronisationUserEvictedFromRoom.FindOrAdd(InRoomId, TSet<FRoomUserId>());
        EvictionList.Add(InRoomMemberId);
    }

    virtual TSet<FRoomUserId> GetMembersEvictedFromRoom(const FRoomIdRef &InRoomId) override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return this->SynchronisationUserEvictedFromRoom.FindOrAdd(InRoomId, TSet<FRoomUserId>());
    }

    virtual void RecordSynchronisationDifference(
        const FRoomRef &InRoom,
        const FSynchronisationDifferenceRef &InSynchronisationDifference) override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        if (!this->RoomId->IsEqual(InRoom->GetRoomId()))
        {
            return;
        }
        this->SynchronisationDifferences.Add(InSynchronisationDifference);
    }

    virtual TArray<FSynchronisationDifferenceRef> GetSynchronisationDifferences(const FRoomRef &InRoom) override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        if (!this->RoomId->IsEqual(InRoom->GetRoomId()))
        {
            return TArray<FSynchronisationDifferenceRef>();
        }
        return this->SynchronisationDifferences;
    }

    virtual void AddAnticipatedKick(
        const FRoomIdRef &InRoomId,
        const FRoomUserId &InLocalUserId,
        const FRoomUserId &InTargetUserId) override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        if (!this->RoomId->IsEqual(InRoomId))
        {
            return;
        }

        auto Value =
            CAnticipatedKick.Create(FRoomIdRefAndLocalUserIdAndTargetUserId(InRoomId, InLocalUserId, InTargetUserId));
        auto ExistingRow = this->AnticipatedKickTable.FindFirstByValue(Value);
        if (!ExistingRow.IsValid())
        {
            this->AnticipatedKickTable.AddRow({Value});
        }
    }

    virtual void RemoveAnticipatedKick(
        const FRoomIdRef &InRoomId,
        const FRoomUserId &InLocalUserId,
        const FRoomUserId &InTargetUserId) override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        if (!this->RoomId->IsEqual(InRoomId))
        {
            return;
        }

        auto Value =
            CAnticipatedKick.Create(FRoomIdRefAndLocalUserIdAndTargetUserId(InRoomId, InLocalUserId, InTargetUserId));
        this->AnticipatedKickTable.RemoveRow(this->AnticipatedKickTable.FindFirstByValue(Value));
    }

    virtual bool HasAnticipatedKick(
        const FRoomIdRef &InRoomId,
        const FRoomUserId &InLocalUserId,
        const FRoomUserId &InTargetUserId) const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        if (!this->RoomId->IsEqual(InRoomId))
        {
            return false;
        }

        auto Value =
            CAnticipatedKick.Create(FRoomIdRefAndLocalUserIdAndTargetUserId(InRoomId, InLocalUserId, InTargetUserId));
        return this->AnticipatedKickTable.FindFirstByValue(Value).IsValid();
    }

    virtual void RemoveAnticipatedKicksExcept(
        const FRoomIdRef &InRoomId,
        const FRoomUserId &InLocalUserId,
        const TSet<FRoomUserId> &InCurrentMembers) override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        for (const auto &Row : this->AnticipatedKickTable.FindByPredicate(
                 [InRoomId, InLocalUserId, InCurrentMembers](const FIndexedRowConstRef &Row) {
                     REDPOINT_EOS_USE_LLM_TAG(Rooms);

                     auto AnticipatedKick = CAnticipatedKick.Get(Row);
                     return AnticipatedKick.Get<0>()->ToString() == InRoomId->ToString() &&
                            AnticipatedKick.Get<1>() == InLocalUserId &&
                            !InCurrentMembers.Contains(AnticipatedKick.Get<2>());
                 }))
        {
            this->AnticipatedKickTable.RemoveRow(Row);
        }
    }
};

void FRoomSystemImpl::SynchroniseSingleRoomImmediately(
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately return.
    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    if (!this->RoomDatabase->HasRoom(Room))
    {
        // This room is no longer tracked. This can happen if an event callback occurs after the room has been left.
        // Don't attempt to synchronise this room (the call to GetRoomEventContextForRoom will check, and we definitely
        // don't want to raise any events).
        return;
    }

    // Create a synchronisation context just for this room. This ensures
    // that we don't affect any other accumulated differences in the
    // room synchronisation (e.g. by clearing the state used for platform
    // ticks).
    auto SingleRoomSynchronisationContext =
        MakeShared<FSingleRoomSynchronisationContext>(Room->GetRoomId(), this->AnticipatedKickTable);

    // For the provider, synchronise the room.
    TArray<FRoomRef> Rooms;
    Rooms.Add(Room);
    RoomProvider->SynchroniseRooms(Rooms, SingleRoomSynchronisationContext);

    // Raise events where necessary.
    this->ComputeSynchronisationDifferencesAndRaiseEvents(
        Room,
        SingleRoomSynchronisationContext->SynchronisationDifferences,
        GetRoomEventContextForRoom(Room));
}

void FRoomSystemImpl::SynchroniseRoomsAfterPlatformTick()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    // If the platform handle has been shutdown, immediately return.
    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    // For each room provider, collect all of the rooms.
    TMap<FName, TArray<FRoomRef>> RoomsByProvider;
    TMap<FRoomRef, FRoomEventContext> RoomEventContexts;
    auto AddRoom = [&RoomsByProvider, &RoomEventContexts](const FRoomRef &Room, const FRoomEventContext &EventContext) {
        FName ProviderName = Room->GetRoomId()->GetProviderName();
        TArray<FRoomRef> &Rooms = RoomsByProvider.FindOrAdd(ProviderName);
        Rooms.Add(Room);
        RoomEventContexts.FindOrAdd(Room, EventContext);
    };
    for (const auto &RoomPair : this->RoomDatabase->GetAllRooms())
    {
        if (RoomPair.Key.Get<0>() == ERoomScope::Global)
        {
            AddRoom(RoomPair.Value, FRoomEventContext::Create<ERoomScope::Global>());
        }
        else
        {
            AddRoom(RoomPair.Value, FRoomEventContext::Create<ERoomScope::User>(RoomPair.Key.Get<1>()));
        }
    }

    // For each provider, synchronise all of the rooms.
    for (const auto &KV : this->RoomProviders)
    {
        TArray<FRoomRef> *Rooms = RoomsByProvider.Find(KV.Key);
        if (Rooms != nullptr)
        {
            KV.Value->SynchroniseRooms(*Rooms, this->AsShared());
        }
    }

    // Take a snapshot of the recorded differences and clear maps, since
    // we're about to call into user code which might result in the EOS
    // SDK firing more events.
    auto SynchronisationDifferencesSnapshot = this->SynchronisationDifferences;
    this->SynchronisationEvents.Empty();
    this->SynchronisationDifferences.Empty();
    this->SynchronisationUserEvictedFromRoom.Empty();

    // For each room, raise events based on the recorded differences.
    for (const auto &KV : SynchronisationDifferencesSnapshot)
    {
        const auto &RoomRef = KV.Key;
        const auto &Differences = KV.Value;
        const auto &EventContext = RoomEventContexts[RoomRef];

        if (!this->RoomDatabase->HasRoom(RoomRef))
        {
            // This room is no longer tracked. I'm not sure it's possible to hit this in
            // SynchroniseRoomsAfterPlatformTick, but guard against a potential check() firing anyway and ignore the
            // room.
            continue;
        }

        this->ComputeSynchronisationDifferencesAndRaiseEvents(RoomRef, Differences, EventContext);
    }
}

Synchronisation::FSynchronisationContextRef FRoomSystemImpl::GetSynchronisationContext()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->AsShared();
}

void FRoomSystemImpl::AddSynchronisationEvent(
    const FRoomIdRef &InRoomId,
    const Synchronisation::FSynchronisationEventRef &InEvent)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    if (!this->SynchronisationEvents.Contains(InRoomId))
    {
        this->SynchronisationEvents.Add(InRoomId, TArray<Synchronisation::FSynchronisationEventRef>());
    }

    this->SynchronisationEvents[InRoomId].Add(InEvent);
}

TArray<Synchronisation::FSynchronisationEventRef> FRoomSystemImpl::GetSynchronisationEventOfTypeBase(
    const FRoomIdRef &InRoomId,
    FName InEventType) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return TArray<Synchronisation::FSynchronisationEventRef>();
    }

    if (!this->SynchronisationEvents.Contains(InRoomId))
    {
        return TArray<Synchronisation::FSynchronisationEventRef>();
    }

    TArray<Synchronisation::FSynchronisationEventRef> Results;
    for (const auto &Event : this->SynchronisationEvents[InRoomId])
    {
        if (Event->GetEventType() == InEventType)
        {
            Results.Add(Event);
        }
    }
    return Results;
}

void FRoomSystemImpl::RecordMemberEvictedFromRoom(const FRoomIdRef &InRoomId, const FRoomUserId &InRoomMemberId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    auto &EvictionList = this->SynchronisationUserEvictedFromRoom.FindOrAdd(InRoomId, TSet<FRoomUserId>());
    EvictionList.Add(InRoomMemberId);
}

TSet<FRoomUserId> FRoomSystemImpl::GetMembersEvictedFromRoom(const FRoomIdRef &InRoomId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return TSet<FRoomUserId>();
    }

    return this->SynchronisationUserEvictedFromRoom.FindOrAdd(InRoomId, TSet<FRoomUserId>());
}

void FRoomSystemImpl::RecordSynchronisationDifference(
    const FRoomRef &InRoom,
    const Synchronisation::FSynchronisationDifferenceRef &InSynchronisationDifference)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    if (!this->SynchronisationDifferences.Contains(InRoom))
    {
        this->SynchronisationDifferences.Add(InRoom, TArray<Synchronisation::FSynchronisationDifferenceRef>());
    }

    this->SynchronisationDifferences[InRoom].Add(InSynchronisationDifference);
}

TArray<Synchronisation::FSynchronisationDifferenceRef> FRoomSystemImpl::GetSynchronisationDifferences(
    const FRoomRef &InRoom)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return TArray<Synchronisation::FSynchronisationDifferenceRef>();
    }

    if (!this->SynchronisationDifferences.Contains(InRoom))
    {
        return TArray<Synchronisation::FSynchronisationDifferenceRef>();
    }

    return this->SynchronisationDifferences[InRoom];
}

void FRoomSystemImpl::AddAnticipatedKick(
    const FRoomIdRef &RoomId,
    const FRoomUserId &LocalUserId,
    const FRoomUserId &TargetUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    UE_LOG(LogRedpointEOSRooms, Verbose, TEXT("Recording anticipated kick of target user."));

    auto Value = CAnticipatedKick.Create(FRoomIdRefAndLocalUserIdAndTargetUserId(RoomId, LocalUserId, TargetUserId));
    auto ExistingRow = this->AnticipatedKickTable.FindFirstByValue(Value);
    if (!ExistingRow.IsValid())
    {
        this->AnticipatedKickTable.AddRow({Value});
    }
}

void FRoomSystemImpl::RemoveAnticipatedKick(
    const FRoomIdRef &RoomId,
    const FRoomUserId &LocalUserId,
    const FRoomUserId &TargetUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    UE_LOG(LogRedpointEOSRooms, Verbose, TEXT("Removing anticipated kick of target user."));

    auto Value = CAnticipatedKick.Create(FRoomIdRefAndLocalUserIdAndTargetUserId(RoomId, LocalUserId, TargetUserId));
    this->AnticipatedKickTable.RemoveRow(this->AnticipatedKickTable.FindFirstByValue(Value));
}

bool FRoomSystemImpl::HasAnticipatedKick(
    const FRoomIdRef &RoomId,
    const FRoomUserId &LocalUserId,
    const FRoomUserId &TargetUserId) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return false;
    }

    UE_LOG(LogRedpointEOSRooms, Verbose, TEXT("Returning anticipated kick of target user status."));

    auto Value = CAnticipatedKick.Create(FRoomIdRefAndLocalUserIdAndTargetUserId(RoomId, LocalUserId, TargetUserId));
    return this->AnticipatedKickTable.FindFirstByValue(Value).IsValid();
}

void FRoomSystemImpl::RemoveAnticipatedKicksExcept(
    const FRoomIdRef &RoomId,
    const FRoomUserId &LocalUserId,
    const TSet<FRoomUserId> &CurrentMembers)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    for (const auto &Row : this->AnticipatedKickTable.FindByPredicate(
             [RoomId, LocalUserId, CurrentMembers](const FIndexedRowConstRef &Row) {
                 REDPOINT_EOS_USE_LLM_TAG(Rooms);

                 auto AnticipatedKick = CAnticipatedKick.Get(Row);
                 return AnticipatedKick.Get<0>()->ToString() == RoomId->ToString() &&
                        AnticipatedKick.Get<1>() == LocalUserId && !CurrentMembers.Contains(AnticipatedKick.Get<2>());
             }))
    {
        this->AnticipatedKickTable.RemoveRow(Row);
    }
}

void FRoomSystemImpl::OnRoomInviteReceivedFromRuntimePlatform(
    const FRoomUserId &LocalUserId,
    const FUserInfoPtr &SenderUser,
    const FRoomIdRef &RoomId,
    const FName &RoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    using namespace ::Redpoint::EOS::Core::Id;

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    UE_LOG(
        LogRedpointEOSRooms,
        Verbose,
        TEXT("Local user '%s' received room invite to room ID '%s' via room advertisement on runtime platform."),
        *GetUserIdString(LocalUserId),
        *RoomId->ToString());

    UE_LOG(
        LogRedpointEOSRooms,
        Verbose,
        TEXT("Searching for room ID '%s' in namespace '%s' on provider '%s' to handle room invite received via room "
             "advertisement."),
        *RoomId->ToString(),
        *RoomNamespace.ToString(),
        *RoomId->GetProviderName().ToString());

    auto SearchRequest = FSearchRoomsRequest(LocalUserId, RoomId->GetProviderName(), RoomNamespace, RoomId);
    SearchRequest.MaxResultCount = 1;
    SearchRequest.bDurable = true;
    this->SearchRooms(
        SearchRequest,
        IRoomSystem::FOnSearchRoomsComplete::CreateSPLambda(
            this,
            [this, RoomId, SenderUser, LocalUserId](
                const FError &ErrorCode,
                const TArray<FRoomSearchResultRef> &RoomSearchResults) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // Propagate error to logs if we can't find the invite.
                if (!ErrorCode.WasSuccessful() || RoomSearchResults.Num() != 1)
                {
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Error,
                        TEXT("Room invite to room ID '%s' from room advertisement on runtime platform could not be "
                             "resolved to a search result for receive: %s"),
                        *RoomId->ToString(),
                        *ErrorCode.ToLogString());
                    return;
                }

                // Create the room invite.
                auto RoomInvite = MakeShared<FRoomInviteImpl>(
                    FString::Printf(TEXT("SyntheticRoomInvite%d"), this->NextSyntheticInviteId++),
                    SenderUser.IsValid() ? SenderUser->GetUserId() : LocalUserId,
                    LocalUserId,
                    RoomSearchResults[0]);

                // Add the invite to the room invites list.
                auto &SpecificUserRoomInvites =
                    this->UserRoomInvites.FindOrAdd(RoomInvite->GetRoomInviteRecipientUserId());
                if (SpecificUserRoomInvites.Contains(RoomInvite->GetRoomInviteId()))
                {
                    // This invite is already tracked. This can happen if RestoreRoomInvites is called multiple times.
                    return;
                }
                SpecificUserRoomInvites.Add(RoomInvite->GetRoomInviteId(), RoomInvite);

                // Log that we received an invite.
                UE_LOG(
                    LogRedpointEOSRooms,
                    Verbose,
                    TEXT("Room invite for room ID '%s' for user ID '%s' was received (from room advertisement on "
                         "runtime platform)."),
                    *RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->ToString(),
                    *GetUserIdString(RoomInvite->GetRoomInviteRecipientUserId()));

                // Broadcast the event to notify users of the room system.
                this->OnRoomInviteReceived(RoomInvite->GetRoomInviteSearchResult()->GetRoomNamespace())
                    .Broadcast(FRoomEventContext::Create(RoomInvite->GetRoomInviteRecipientUserId()), RoomInvite);
            }));
}

void FRoomSystemImpl::OnRoomInviteAcceptedFromRuntimePlatform(
    const FRoomUserId &LocalUserId,
    const FUserInfoPtr &SenderUser,
    const FRoomIdRef &RoomId,
    const FName &RoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    using namespace ::Redpoint::EOS::Core::Id;

    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    UE_LOG(
        LogRedpointEOSRooms,
        Verbose,
        TEXT("Local user '%s' accepted room invite to room ID '%s' via room advertisement on runtime platform."),
        *GetUserIdString(LocalUserId),
        *RoomId->ToString());

    UE_LOG(
        LogRedpointEOSRooms,
        Verbose,
        TEXT("Searching for room ID '%s' in namespace '%s' on provider '%s' to handle room invite acceptance via room "
             "advertisement."),
        *RoomId->ToString(),
        *RoomNamespace.ToString(),
        *RoomId->GetProviderName().ToString());

    auto SearchRequest = FSearchRoomsRequest(LocalUserId, RoomId->GetProviderName(), RoomNamespace, RoomId);
    SearchRequest.MaxResultCount = 1;
    SearchRequest.bDurable = true;
    this->SearchRooms(
        SearchRequest,
        IRoomSystem::FOnSearchRoomsComplete::CreateSPLambda(
            this,
            [this, RoomId, SenderUser, LocalUserId](
                const FError &ErrorCode,
                const TArray<FRoomSearchResultRef> &RoomSearchResults) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms);

                // Propagate error to logs if we can't find the invite.
                if (!ErrorCode.WasSuccessful() || RoomSearchResults.Num() != 1)
                {
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Error,
                        TEXT("Room invite to room ID '%s' from room advertisement on runtime platform could not be "
                             "resolved to a search result for acceptance: %s"),
                        *RoomId->ToString(),
                        *ErrorCode.ToLogString());
                    return;
                }

                // Create the room invite.
                auto RoomInvite = MakeShared<FRoomInviteImpl>(
                    FString::Printf(TEXT("SyntheticRoomInvite%d"), this->NextSyntheticInviteId++),
                    SenderUser.IsValid() ? SenderUser->GetUserId() : LocalUserId,
                    LocalUserId,
                    RoomSearchResults[0]);

                // Log that we accepted an invite.
                UE_LOG(
                    LogRedpointEOSRooms,
                    Verbose,
                    TEXT("Room invite for room ID '%s' for user ID '%s' was accepted (from room advertisement on "
                         "runtime platform)."),
                    *RoomInvite->GetRoomInviteSearchResult()->GetRoomId()->ToString(),
                    *GetUserIdString(RoomInvite->GetRoomInviteRecipientUserId()));

                // We need to propagate this out of the room system so that external systems can handle it. This is
                // because at this point, we don't know the room scope or room features to attach.
                this->OnRoomJoinRequestedDelegate.Broadcast(
                    RoomInvite->GetRoomInviteSearchResult()->GetRoomNamespace(),
                    FRoomEventContext::Create(RoomInvite->GetRoomInviteRecipientUserId()),
                    MakeShared<FRoomRequestedJoinImpl>(RoomInvite));
            }));
}

void FRoomSystemImpl::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &Commands)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

#if UE_ALLOW_EXEC_COMMANDS
    if (this->PlatformHandle->IsShutdown())
    {
        return;
    }

    FAutoCompleteCommand Dump;
    Dump.Command = TEXT("Dump");
    Dump.Desc = TEXT("Dump the full state of the room system to the logs and command output.");
    Commands.Add(Dump);
#endif
}

#if REDPOINT_EOS_UE_5_3_OR_LATER

bool FRoomSystemImpl::Exec_Runtime(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    if (this->PlatformHandle->IsShutdown())
    {
        return false;
    }

    if (FParse::Command(&Cmd, TEXT("Dump"), false))
    {
        auto Rooms = this->RoomDatabase->GetAllRooms();
        Ar.Logf(ELogVerbosity::Verbose, TEXT("Tracking %d rooms."), Rooms.Num());
        for (const auto &RoomTuple : Rooms)
        {
            auto RoomScopeAndUserId = RoomTuple.Get<0>();
            auto Room = RoomTuple.Get<1>();

            if (RoomScopeAndUserId.Get<0>() == ERoomScope::Global)
            {
                Ar.Logf(ELogVerbosity::Verbose, TEXT("%s (global scope):"), *Room->GetRoomId()->ToString());
            }
            else
            {
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("%s (user scope for %s):"),
                    *Room->GetRoomId()->ToString(),
                    *GetUserIdDebugString(RoomScopeAndUserId.Get<1>()));
            }

            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("  Bound for local user: %s"),
                *GetUserIdDebugString(Room->GetBoundForLocalUser()));
            Ar.Logf(ELogVerbosity::Verbose, TEXT("  Namespace: %s"), *Room->GetRoomNamespace().ToString());
            Ar.Logf(
                ELogVerbosity::Verbose,
                TEXT("  Owner: %s"),
                Room->GetRoomOwner()->IsDedicatedServer()
                    ? *FString::Printf(TEXT("server '%s'"), *Room->GetRoomOwner()->GetDedicatedServerClientId())
                    : *FString::Printf(TEXT("user '%s'"), *GetUserIdDebugString(Room->GetRoomOwner()->GetUserId())));
            Ar.Logf(ELogVerbosity::Verbose, TEXT("  Maximum members: %u"), Room->GetRoomMaxMembers());
            Ar.Logf(ELogVerbosity::Verbose, TEXT("  Attributes: (%d attributes)"), Room->GetRoomAttributes().Num());
            for (const auto &KV : Room->GetRoomAttributes())
            {
                switch (KV.Value.Type())
                {
                case FRoomAttribute::EType::String:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("    %s: %s"), *KV.Key, *KV.Value.GetString());
                    break;
                case FRoomAttribute::EType::Double:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("    %s: %f"), *KV.Key, KV.Value.GetDouble());
                    break;
                case FRoomAttribute::EType::Int64:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("    %s: %d"), *KV.Key, KV.Value.GetInt64());
                    break;
                case FRoomAttribute::EType::Bool:
                    Ar.Logf(
                        ELogVerbosity::Verbose,
                        TEXT("    %s: %s"),
                        *KV.Key,
                        KV.Value.GetBool() ? TEXT("true") : TEXT("false"));
                    break;
                default:
                    Ar.Logf(ELogVerbosity::Verbose, TEXT("    %s: (unknown)"), *KV.Key);
                    break;
                }
            }
            Ar.Logf(ELogVerbosity::Verbose, TEXT("  Features: (%d features)"), Room->GetRoomFeatureNames().Num());
            for (const auto &FeatureName : Room->GetRoomFeatureNames())
            {
                Ar.Logf(ELogVerbosity::Verbose, TEXT("    %s"), *FeatureName.ToString());
            }
            Ar.Logf(ELogVerbosity::Verbose, TEXT("  Members: (%d members)"), Room->GetMembers().Num());
            for (const auto &Member : Room->GetMembers())
            {
                Ar.Logf(
                    ELogVerbosity::Verbose,
                    TEXT("    %s: (%d member attributes)"),
                    *GetUserIdDebugString(Member->GetUserId()),
                    Member->GetAttributes().Num());
                for (const auto &KV : Member->GetAttributes())
                {
                    switch (KV.Value.Type())
                    {
                    case FRoomAttribute::EType::String:
                        Ar.Logf(ELogVerbosity::Verbose, TEXT("      %s: %s"), *KV.Key, *KV.Value.GetString());
                        break;
                    case FRoomAttribute::EType::Double:
                        Ar.Logf(ELogVerbosity::Verbose, TEXT("      %s: %f"), *KV.Key, KV.Value.GetDouble());
                        break;
                    case FRoomAttribute::EType::Int64:
                        Ar.Logf(ELogVerbosity::Verbose, TEXT("      %s: %d"), *KV.Key, KV.Value.GetInt64());
                        break;
                    case FRoomAttribute::EType::Bool:
                        Ar.Logf(
                            ELogVerbosity::Verbose,
                            TEXT("      %s: %s"),
                            *KV.Key,
                            KV.Value.GetBool() ? TEXT("true") : TEXT("false"));
                        break;
                    default:
                        Ar.Logf(ELogVerbosity::Verbose, TEXT("      %s: (unknown)"), *KV.Key);
                        break;
                    }
                }
            }
        }

        return true;
    }

    return false;
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()