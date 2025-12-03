// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Exec/ExecWithAutoComplete.h"
#include "RedpointEOSCore/Id/AccountIdMap.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSCore/Utils/IndexedTable.h"
#include "RedpointEOSCore/Utils/NamespacedEvent.h"
#include "RedpointEOSRooms/Database/RoomDatabase.h"
#include "RedpointEOSRooms/Providers/RoomProvider.h"
#include "RedpointEOSRooms/RoomExclusivityDelegates.h"
#include "RedpointEOSRooms/RoomIdAndAliasMap.h"
#include "RedpointEOSRooms/RoomIdMap.h"
#include "RedpointEOSRooms/RoomInviteIdMap.h"
#include "RedpointEOSRooms/RoomLeaveReason.h"
#include "RedpointEOSRooms/RoomMap.h"
#include "RedpointEOSRooms/RoomSystem.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationContext.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(293683484, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Database;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSROOMS_API IRoomSystemImpl : public IRoomSystem
{
public:
    IRoomSystemImpl() = default;
    UE_NONCOPYABLE(IRoomSystemImpl);
    virtual ~IRoomSystemImpl() override = default;

    /**
     * Applies recorded synchronisation events to all of the known rooms. This must occur
     * after the EOS SDK platform has been ticked.
     */
    virtual void SynchroniseRoomsAfterPlatformTick() = 0;

    /**
     * Get the synchronisation context which native event callbacks can add events to.
     */
    virtual Synchronisation::FSynchronisationContextRef GetSynchronisationContext() = 0;
};

class REDPOINTEOSROOMS_API FRoomSystemImpl : public IRoomSystemImpl,
                                             public Synchronisation::ISynchronisationContext,
                                             public IHasEventRegistration,
                                             public FExecWithAutoComplete,
                                             public TSharedFromThis<FRoomSystemImpl>
{
private:
    FPlatformHandle PlatformHandle;
    TNamespacedEvent<IRoomSystem::FOnRoomJoined> OnRoomJoinedDelegate;
    TNamespacedEvent<IRoomSystem::FOnRoomLeft> OnRoomLeftDelegate;
    TNamespacedEvent<IRoomSystem::FOnRoomUpdated> OnRoomUpdatedDelegate;
    TNamespacedEvent<IRoomSystem::FOnRoomInviteReceived> OnRoomInviteReceivedDelegate;
    TNamespacedEvent<IRoomSystem::FOnRoomInviteRemoved> OnRoomInviteRemovedDelegate;
    TNamespacedEvent<IRoomSystem::FOnRoomJoinRequested> OnRoomJoinRequestedDelegate;
    TNamespacedEvent<IRoomSystem::FOnRoomLeaveRequested> OnRoomLeaveRequestedDelegate;
    TRoomIdMap<TArray<Synchronisation::FSynchronisationEventRef>> SynchronisationEvents;
    TRoomMap<TArray<Synchronisation::FSynchronisationDifferenceRef>> SynchronisationDifferences;
    TArray<FRoomRef> ExplicitlyLeavingRooms;
    TRoomIdMap<TSet<FRoomUserId>> SynchronisationUserEvictedFromRoom;
    TSharedRef<FRoomDatabase> RoomDatabase;
    Core::Id::TAccountIdMap<TRoomInviteIdMap<FRoomInviteRef>> UserRoomInvites;
    TMap<FName, TSharedPtr<FRoomProvider>> RoomProviders;
    FIndexedTable AnticipatedKickTable;
    int32 NextSyntheticInviteId;

    FRoomEventContext GetRoomEventContextForRoom(const FRoomRef &Room) const;

    void BroadcastOrDeferRoomEvent(
        const FRoomEventContext &RoomContext,
        const FRoomRef &Room,
        const FSimpleDelegate &EventBroadcast);

public:
    FRoomSystemImpl(const FPlatformHandle &InPlatformHandle);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FRoomSystemImpl);
    virtual ~FRoomSystemImpl() override = default;

private:
    void OnPlatformShutdown(FPlatformInstance &);

    void OnRoomJoinRequestedFromSDKEvent(FRoomUserId ActorUserId, FRoomSearchResultRef RoomSearchResult);
    void OnRoomLeaveRequestedFromSDKEvent(FRoomUserId ActorUserId, FRoomIdRef RoomId);
    void OnRoomInviteLocallyReceivedFromSDKEvent(
        FRoomInviteId RoomInviteId,
        FRoomUserId RoomInviteSenderUserId,
        FRoomUserId RoomInviteLocalRecipientUserId,
        FRoomSearchResultRef RoomSearchResult);
    void OnRoomInviteLocallyFullyRejectedFromSDKEvent(
        FRoomInviteId RoomInviteId,
        FRoomUserId RoomInviteLocalRecipientUserId);

    void AutoLeaveRoom(int TransactionId, const FRoomRef &Room, const FRoomDatabase::FRoomLeft &RoomLeft);

public:
    virtual TArray<FRoomRef> GetCurrentRooms(const FGetCurrentRoomsRequest &Request) override;
    virtual void CreateRoom(const FCreateRoomRequest &Request, const FOnCreateRoomComplete &OnComplete) override;
    virtual void UpdateRoom(
        const FRoomRef &Room,
        const FUpdateRoomRequest &Request,
        const FOnUpdateRoomComplete &OnComplete) override;
    virtual void SearchRooms(const FSearchRoomsRequest &Request, const FOnSearchRoomsComplete &OnComplete) override;
    virtual void JoinRoom(const FJoinRoomRequest &Request, const FOnJoinRoomComplete &OnComplete) override;

private:
    void JoinRoomWithSearchResult(
        const FJoinRoomRequest &Request,
        const TSharedPtr<FRoomProvider> &RoomProvider,
        const FRoomSearchResultRef &SearchResult,
        const FOnJoinRoomComplete &OnComplete);

public:
    virtual void LeaveRoom(
        const FRoomRef &Room,
        const FLeaveRoomRequest &Request,
        const FOnLeaveRoomComplete &OnComplete) override;
    virtual void PromoteRoomMember(
        const FRoomRef &Room,
        const FPromoteRoomMemberRequest &Request,
        const FOnPromoteRoomMemberComplete &OnComplete) override;
    virtual void KickRoomMember(
        const FRoomRef &Room,
        const FKickRoomMemberRequest &Request,
        const FOnKickRoomMemberComplete &OnComplete) override;
    virtual FOnRoomJoined &OnRoomJoined(const FName &InRoomNamespace) override;
    virtual FOnRoomLeft &OnRoomLeft(const FName &InRoomNamespace) override;
    virtual FOnRoomUpdated &OnRoomUpdated(const FName &InRoomNamespace) override;
    virtual TArray<FRoomInviteRef> GetCurrentRoomInvites(const FRoomUserId &InLocalUser, const FName &InRoomNamespace)
        override;
    virtual void RestoreRoomInvites(
        const FRestoreRoomInvitesRequest &Request,
        const FOnRestoreRoomInvitesComplete &OnComplete) override;
    virtual void SendRoomInvite(
        const FRoomRef &Room,
        const FSendRoomInviteRequest &Request,
        const FOnSendRoomInviteComplete &OnComplete) override;
    virtual void RejectRoomInvite(const FRoomInviteRef &RoomInvite, const FOnRejectRoomInviteComplete &OnComplete)
        override;
    virtual FOnRoomInviteReceived &OnRoomInviteReceived(const FName &InRoomNamespace) override;
    virtual FOnRoomInviteRemoved &OnRoomInviteRemoved(const FName &InRoomNamespace) override;
    virtual FOnRoomJoinRequested &OnRoomJoinRequested(const FName &InRoomNamespace) override;
    virtual FOnRoomLeaveRequested &OnRoomLeaveRequested(const FName &InRoomNamespace) override;
    virtual bool HasRoomProvider(const FName &InProviderName) override;
    virtual void RejectRoomRequestedJoin(
        const FRoomRequestedJoinRef &RoomRequestedJoin,
        const FOnRejectRoomRequestedJoinComplete &OnComplete) override;

private:
    void ComputeSynchronisationDifferencesAndRaiseEvents(
        const FRoomRef &RoomRef,
        const TArray<Synchronisation::FSynchronisationDifferenceRef> &Differences,
        const FRoomEventContext &EventContext);
    void SynchroniseSingleRoomImmediately(const TSharedRef<FRoomProvider> &RoomProvider, const FRoomRef &Room);

public:
    virtual void SynchroniseRoomsAfterPlatformTick() override;
    virtual Synchronisation::FSynchronisationContextRef GetSynchronisationContext() override;

    virtual void AddSynchronisationEvent(
        const FRoomIdRef &InRoomId,
        const Synchronisation::FSynchronisationEventRef &InEvent) override;
    virtual TArray<Synchronisation::FSynchronisationEventRef> GetSynchronisationEventOfTypeBase(
        const FRoomIdRef &InRoomId,
        FName InEventType) const override;
    virtual void RecordMemberEvictedFromRoom(const FRoomIdRef &InRoomId, const FRoomUserId &InRoomMemberId) override;
    virtual TSet<FRoomUserId> GetMembersEvictedFromRoom(const FRoomIdRef &InRoomId) override;
    virtual void RecordSynchronisationDifference(
        const FRoomRef &InRoom,
        const Synchronisation::FSynchronisationDifferenceRef &InSynchronisationDifference) override;
    virtual TArray<Synchronisation::FSynchronisationDifferenceRef> GetSynchronisationDifferences(
        const FRoomRef &InRoom) override;
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

public:
    virtual void OnRoomInviteReceivedFromRuntimePlatform(
        const FRoomUserId &LocalUserId,
        const FUserInfoPtr &SenderUser,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace) override;
    virtual void OnRoomInviteAcceptedFromRuntimePlatform(
        const FRoomUserId &LocalUserId,
        const FUserInfoPtr &SenderUser,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace) override;
    virtual void OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent(
        FRoomInviteId RoomInviteId,
        FRoomUserId RoomInviteLocalRecipientUserId) override;

    virtual void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &) override;
#if REDPOINT_EOS_UE_5_3_OR_LATER
    virtual bool Exec_Runtime(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar) override;
#endif
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(293683484, Redpoint::EOS::Rooms, IRoomSystemImpl)
REDPOINT_EOS_FILE_NS_EXPORT(293683484, Redpoint::EOS::Rooms, FRoomSystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()