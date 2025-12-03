// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSCore/Exec/ExecWithAutoComplete.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSCore/Utils/PeriodicTimer.h"
#include "RedpointEOSIdentity/IdentityUser.h"
#include "RedpointEOSPresence/PresenceMessageGlobalConnection.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSPresence/PresenceSystemDeferredInterestCallback.h"
#include "RedpointEOSPresence/PresenceSystemLocalUserState.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSRooms/RoomSystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3969572298, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;

class FPresenceSystemImpl : public IPresenceSystem,
                            public IHasEventRegistration,
                            public FExecWithAutoComplete,
                            public TSharedFromThis<FPresenceSystemImpl>
{
private:
    FPlatformHandle PlatformHandle;
    TMap<FIdentityUserId, TSharedPtr<FPresenceSystemLocalUserState>> LocalUserStates;
    TSharedRef<FExclusiveAsyncOperationQueue> SyncQueue;
    TSharedRef<FPeriodicTimer> LocalTimer;
    TSharedRef<FPeriodicTimer> RemoteTimer;
    TMap<FName, FOnRemoteUserPresenceStateUpdated> RemoteUserPresenceStateUpdatedDelegates;
    TMap<FName, FOnInboundMessageConnectionEstablished> InboundMessageConnectionEstablishedDelegates;
    FDelegateHandle OnRoomJoinedHandle;
    FDelegateHandle OnRoomUpdatedHandle;
    FDelegateHandle OnRoomLeftHandle;
    TArray<FPresenceSystemDeferredInterestCallback> DeferredInterestCallbacks;
    int PendingRemoteRefreshCallbacks;
    FOnInternetConnectivityChanged InternetConnectivityChangedDelegate;
    bool bDisconnectedFromInternet;

    static constexpr float PeriodicSecondsForLocalUserCheck = 1.0f;
#if WITH_EDITOR
    static constexpr float PeriodicSecondsForRemoteUserRefresh = 10.0f;
    static constexpr double MinimumAgeForRemoteUserRefresh = 10.0;
#else
    static constexpr float PeriodicSecondsForRemoteUserRefresh = 30.0f;
    static constexpr double MinimumAgeForRemoteUserRefresh = 30.0;
#endif
    static constexpr int MaximumProductUserIdsPerRefresh = 20;
    static constexpr int MaximumAttributeValueFilterLength = 2000;

public:
    FPresenceSystemImpl(const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FPresenceSystemImpl);
    virtual ~FPresenceSystemImpl() override = default;

    virtual void RegisterEvents() override;
    void UnregisterEvents();

    typedef TDelegate<void(const FError &ErrorCode)> FOnLocalUserAdded;
    typedef TDelegate<void()> FOnLocalUserRemoved;

    void AddLocalUser(const FIdentityUserRef &InLocalUser, const FOnLocalUserAdded &InCompletionDelegate);
    void RemoveLocalUser(const FIdentityUserRef &InLocalUser, const FOnLocalUserRemoved &InCompletionDelegate);

    virtual FOnRemoteUserPresenceStateUpdated &OnRemoteUserPresenceStateUpdated(const FName &InterestedSystem) override;
    virtual void UpdatePresenceInterest(
        const FName &InterestedSystem,
        const FIdentityUserId &LocalUserId,
        const TArray<FPresenceInterestChange> &TargetUserInterest,
        bool bWaitForInitialPresenceData,
        const FOnUpdatePresenceInterestComplete &CompletionDelegate) override;
    virtual TOptional<FPresenceUserState> GetPresence(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &TargetUserId) const override;
    virtual void SetPresence(
        const FIdentityUserId &LocalUserId,
        const FPresenceUserState &NewPresenceState,
        const FOnSetPresenceComplete &CompletionDelegate) override;

private:
    /**
     * Update the bPresenceAdvertised value on the desired presence based on what the runtime integrations indicate.
     */
    void UpdateDesiredPresenceAdvertised(const TSharedPtr<FPresenceSystemLocalUserState> &LocalUserState);

    /**
     * Schedule presence updates for local users where the current presence is out-of-date with the
     * desired presence.
     */
    void ScheduleDesiredPresenceUpdatesWhereOutOfDate();

    /**
     * Synchronise the local user's desired presence state to the backend.
     */
    void OnProcessRequestedLocalUserUpdate(
        const FExclusiveAsyncOperationQueue::FOnDone &InOnDone,
        FIdentityUserId InLocalUserId,
        TSharedRef<FPresenceSystemLocalUserState> InFallbackStateForSignalling,
        uint32 InThisPresenceUpdate);

    /**
     * Queue local user's desired presence state for synchronisation when they join a room.
     */
    void OnRoomJoined(const FRoomEventContext &EventContext, const FRoomRef &Room, ERoomJoinReason Reason);

    /**
     * Queue local user's desired presence state for synchronisation when a room they are in is updated.
     */
    void OnRoomUpdated(
        const FRoomEventContext &EventContext,
        const FRoomRef &Room,
        const FRoomUpdateInfo &RoomUpdateInfo);

    /**
     * Queue local user's desired presence state for synchronisation when they leave a room.
     */
    void OnRoomLeft(const FRoomEventContext &EventContext, const FRoomRef &Room, ERoomLeaveReason Reason);

    /**
     * Create the advertised room presence state from the room's current state.
     */
    FPresenceUserAdvertisedRoom CreateAdvertisedRoomFromRoom(const FRoomRef &Room);

    /**
     * Update the desired presence state of all local users when one or more rooms change. We synchronise the presence
     * state of all local users, since some room changes might be for global rooms (i.e. LocalUserId on the room event
     * context of the above handlers will not be set).
     */
    void OnUpdateDesiredPresenceStateForRooms();

    /**
     * Periodically schedule refreshes of remote presence information for local users.
     */
    void ScheduleRemotePresenceRefreshForLocalUsers();

    /**
     * Immediately schedule a refresh of remote presence data for the users that the local user is interested in.
     */
    void ScheduleRemotePresenceRefreshForLocalUser(const FIdentityUserId &LocalUserId);

    /**
     * Refresh a batch of target users that the local user is interested in.
     */
    void OnProcessRequestedRemotePresenceRefresh(
        const FExclusiveAsyncOperationQueue::FOnDone &InOnDone,
        FIdentityUserId InLocalUserId);
    void OnProcessRequestedRemotePresenceRefreshAfterUserInfoPopulated(
        const FExclusiveAsyncOperationQueue::FOnDone &InOnDone,
        FIdentityUserId InLocalUserId);

    /**
     * Invoked by the room system after we've searched for lobbies owned by target users.
     */
    void OnRequestedRoomPresenceRoomsSearched(
        const FError &ErrorCode,
        const TArray<FRoomSearchResultRef> &RoomSearchResults,
        TSet<FIdentityUserId> AccountIdsQueried,
        FExclusiveAsyncOperationQueue::FOnDone InOnDone,
        FIdentityUserId InLocalUserId);

    /**
     * Dispatch any deferred interest callbacks that are now completed.
     */
    void DispatchCompletedDeferredInterestCallbacks();

public:
    virtual FOnInboundMessageConnectionEstablished &OnInboundMessageConnectionEstablished(
        const FName &InterestedSystem) override;
    virtual void TryOpenMessageConnection(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &TargetUserId,
        const FName &InterestedSystem,
        const FOnTryOpenMessageConnection &CompletionDelegate) override;

private:
    void OnRemoteConnectionReceived(
        const TSharedRef<IPresenceMessageConnection> &NewConnection,
        TSharedRef<FPresenceSystemLocalUserState> LocalUserState);

    TSharedRef<IPresenceMessageConnection> CreateAndTrackGlobalConnection(
        const TSharedRef<IRoomSystem> &RoomSystem,
        const FName &InterestedSystem,
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &TargetUserId,
        const FRoomRef &Room);

    void OnSearchResultsForOpenMessageConnection(
        const FError &SearchErrorCode,
        const TArray<FRoomSearchResultRef> &RoomSearchResults,
        FIdentityUserId LocalUserId,
        FIdentityUserId TargetUserId,
        FName InterestedSystem,
        TSharedRef<IRoomSystem> RoomSystem,
        FOnTryOpenMessageConnection CompletionDelegate);

    void OnJoinRoomForOpenMessageConnection(
        const FError &JoinErrorCode,
        const FRoomPtr &JoinedRoom,
        FIdentityUserId LocalUserId,
        FIdentityUserId TargetUserId,
        FName InterestedSystem,
        TSharedRef<IRoomSystem> RoomSystem,
        FOnTryOpenMessageConnection CompletionDelegate);

public:
    virtual TOptional<FString> GetFriendCode(const FIdentityUserId &LocalUserId) override;
    virtual void FindByFriendCode(
        const FIdentityUserId &LocalUserId,
        const FString &FriendCode,
        const FOnFindByFriendCodeComplete &CompletionDelegate) override;

private:
    void OnSearchResultsForFindByFriendCode(
        const FError &SearchErrorCode,
        const TArray<FRoomSearchResultRef> &RoomSearchResults,
        FIdentityUserId LocalUserId,
        TSharedRef<IRoomSystem> RoomSystem,
        FOnFindByFriendCodeComplete CompletionDelegate);

public:
    virtual FOnInternetConnectivityChanged &OnInternetConnectivityChanged() override;

private:
    void RecomputeInternetConnectivityState();

public:
    virtual void WaitForReconciledPresence(
        const FIdentityUserId &LocalUserId,
        const FSimpleDelegate &CompletionDelegate) override;

    virtual void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &) override;
#if REDPOINT_EOS_UE_5_3_OR_LATER
    virtual bool Exec_Runtime(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar) override;
#endif
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(3969572298, Redpoint::EOS::Presence, FPresenceSystemImpl)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()