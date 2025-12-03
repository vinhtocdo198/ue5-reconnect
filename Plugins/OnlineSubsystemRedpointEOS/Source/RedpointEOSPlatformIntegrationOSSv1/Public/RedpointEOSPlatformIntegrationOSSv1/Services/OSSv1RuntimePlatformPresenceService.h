// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Containers/Ticker.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatform/Types/PlatformStorage.h"
#include "RedpointEOSPlatformIntegrationOSSv1/ResolveUserIdToExternalAccountIdInfo.h"
#include "RedpointEOSPresence/PresenceUserActivityState.h"
#include "RedpointEOSPresence/PresenceUserAdvertisedRoom.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSUserCache/UserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1158129887, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSPLATFORMINTEGRATIONOSSV1_API FOSSv1RuntimePlatformPresenceService
    : public IRuntimePlatformPresenceService,
      public IHasEventRegistration,
      public TSharedFromThis<FOSSv1RuntimePlatformPresenceService>
{
private:
    EOS_EExternalAccountType ExternalAccountTypeToHandle;
    FName SubsystemName;
    struct FPlatformStorage
    {
        TMap<int32, FOnRoomAdvertisementInviteReceived> OnRoomAdvertisementInviteReceivedDelegate;
        TMap<int32, FOnRoomAdvertisementInviteAccepted> OnRoomAdvertisementInviteAcceptedDelegate;
        TMap<int32, FOnNonUnifiedFriendPresenceChanged> OnNonUnifiedFriendPresenceChangedDelegate;
        TMap<int32, FDelegateHandle> OnSessionInviteReceivedDelegateHandle;
        TMap<int32, FDelegateHandle> OnSessionInviteAcceptedDelegateHandle;
        FDelegateHandle OnPresenceReceivedDelegateHandle;
        FTSTicker::FDelegateHandle OnTickAfterRestoreInvitesAfterLoginDelegateHandle;
        FDelegateHandle OnConnectivityStateChangedDelegateHandle;

        // These track the last presence info that was set so that it can be restored after temporary loss in Internet
        // connectivity. This code assumes that the OSSv1 implementation and underlying SDK does not preserve presence
        // or advertised rooms when the Internet connection is lost.
        TMap<int32, TTuple<FRuntimePlatformServiceCallContextRef, FPresenceUserState>> LastSetPresence;
        TMap<
            int32,
            TTuple<FRuntimePlatformServiceCallContextRef, TOptional<FPresenceUserAdvertisedRoom>, FRoomIdRef, bool>>
            LastAdvertisedRoom;

        // @note: Some OSSv1 implementations don't correctly populate the LastConnectionState argument, so we have to do
        // our own tracking of the last connection state via Storage.RealLastConnectionState.
        EOnlineServerConnectionStatus::Type RealLastConnectionState;
    };
    TPlatformStorage<FPlatformStorage> PlatformStorage;
    FResolveUserIdToExternalAccountIdInfo ExternalInfoResolver;
    FString FriendsListName;

    /**
     * Some online subsystems do not wait or defer global invites on startup - they just immediately broadcast. We can't
     * wait until the EOS user signs in to capture these via RegisterEvents, so when the target is a game (or an editor
     * running with -Game), we register OnSessionUserInviteAccepted early on the default OSS instance and flush these
     * out of the queue as part of RestoreInvitesAfterLogin.
     */
    TArray<FOnlineSessionSearchResult> QueuedGlobalInvites;

public:
    FOSSv1RuntimePlatformPresenceService(
        const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
        const FName &InSubsystemName,
        const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver,
        const FString &InFriendsListName);
    UE_NONCOPYABLE(FOSSv1RuntimePlatformPresenceService);
    virtual ~FOSSv1RuntimePlatformPresenceService() override = default;

    virtual void RegisterEvents() override;

    virtual TOptional<bool> IsLocalUserOnline(const FRuntimePlatformServiceCallContextRef &CallContext) const override;

protected:
    virtual void WaitForPlatformReadiness(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &InCompletionDelegate);

    virtual void PlatformConfigureSearchForRoomId(FOnlineSessionSearch &Search, const FRoomIdRef &RoomId) const;

    virtual void PlatformAssignRoomIdAndNamespaceToSettings(
        FOnlineSessionSettings &Settings,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace) const;

    virtual void PlatformTryGetRoomIdAndNamespaceFromSession(
        const FOnlineSession &Session,
        TOptional<FRoomIdRef> &OutRoomId,
        TOptional<FName> &OutNamespace) const;

    virtual bool PlatformIsUnsuccessfulSearchStillOk() const;

    virtual bool PlatformSupportsSearch() const;

public:
    virtual void UpdateLocalUserPresence(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserState &InDesiredPresenceState,
        const FOnLocalUserPresenceUpdated &InCompletionDelegate) override;

private:
    void UpdateLocalUserPresenceAfterPlatformReady(
        FRuntimePlatformServiceCallContextRef CallContext,
        FPresenceUserState InDesiredPresenceState,
        FOnLocalUserPresenceUpdated InCompletionDelegate);

public:
    virtual void UpsertRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
        bool bIsParty,
        const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate) override;

private:
    void UpsertRoomAdvertisementWithPoisonFlag(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
        bool bIsParty,
        const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate,
        bool bExistingSessionIsPoisoned);

    void UpsertRoomAdvertisementWithPoisonFlagAfterPlatformReady(
        FRuntimePlatformServiceCallContextRef CallContext,
        FPresenceUserAdvertisedRoom InAdvertisedRoom,
        bool bIsParty,
        FOnUpsertRoomAdvertisementComplete InCompletionDelegate,
        bool bExistingSessionIsPoisoned);

    void OnDestroySessionCompletePriorToSearchComplete(
        FName SessionName,
        bool bWasSuccessful,
        FRuntimePlatformServiceCallContextRef CallContext,
        FPresenceUserAdvertisedRoom AdvertisedRoom,
        bool bIsParty,
        FOnUpsertRoomAdvertisementComplete CompletionDelegate,
        TWeakPtr<IOnlineSession> OnlineSessionWk,
        FUniqueNetIdRef LocalUserId,
        TSharedRef<FOnlineSessionSearch> Search);

public:
    virtual void DeleteRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FRoomIdRef &InRoomId,
        bool bIsParty,
        const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate) override;

private:
    void DeleteRoomAdvertisementAfterPlatformReady(
        FRuntimePlatformServiceCallContextRef CallContext,
        FRoomIdRef InRoomId,
        bool bIsParty,
        FOnDeleteRoomAdvertisementComplete InCompletionDelegate);

public:
    virtual FOnRoomAdvertisementInviteReceived &OnRoomAdvertisementInviteReceived(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;

    virtual FOnRoomAdvertisementInviteAccepted &OnRoomAdvertisementInviteAccepted(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;

    virtual TOptional<EPresenceUserActivityState> GetNonUnifiedFriendPresence(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId) override;

    virtual FOnNonUnifiedFriendPresenceChanged &OnNonUnifiedFriendPresenceChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;

    virtual void SendRoomInviteToNonUnifiedFriend(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId,
        const FRoomIdRef &RoomId,
        const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate) override;

    virtual bool TryOpenRoomInviteRecipientSelection(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FRoomIdRef &RoomId) override;

    void RegisterEventsBeforeLogin(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &InCompletionDelegate);

    void RestoreInvitesAfterLogin(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &InCompletionDelegate);

    void UnregisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext);

private:
    void ApplyAdvertisedRoomToSessionSettings(
        const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
        FOnlineSessionSettings &Settings);

    void OnFindSessionsComplete(
        bool bWasSuccessful,
        FRuntimePlatformServiceCallContextRef CallContext,
        FPresenceUserAdvertisedRoom AdvertisedRoom,
        bool bIsParty,
        FOnUpsertRoomAdvertisementComplete CompletionDelegate,
        TWeakPtr<IOnlineSession> OnlineSessionWk,
        FUniqueNetIdRef LocalUserId,
        TSharedRef<FOnlineSessionSearch> Search);

    void OnCreateSessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        FRuntimePlatformServiceCallContextRef CallContext,
        FPresenceUserAdvertisedRoom AdvertisedRoom,
        bool bIsParty,
        FOnUpsertRoomAdvertisementComplete CompletionDelegate,
        TWeakPtr<IOnlineSession> OnlineSessionWk,
        FName ExpectedSessionName);

    void OnJoinSessionComplete(
        FName SessionName,
        EOnJoinSessionCompleteResult::Type ResultType,
        FRuntimePlatformServiceCallContextRef CallContext,
        FPresenceUserAdvertisedRoom AdvertisedRoom,
        bool bIsParty,
        FOnUpsertRoomAdvertisementComplete CompletionDelegate,
        TWeakPtr<IOnlineSession> OnlineSessionWk,
        FName ExpectedSessionName);

    void OnUpdateSessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        FRuntimePlatformServiceCallContextRef CallContext,
        FPresenceUserAdvertisedRoom AdvertisedRoom,
        bool bIsParty,
        FOnUpsertRoomAdvertisementComplete CompletionDelegate,
        TWeakPtr<IOnlineSession> OnlineSessionWk,
        FName ExpectedSessionName);

    void OnDestroySessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        FRuntimePlatformServiceCallContextRef CallContext,
        FRoomIdRef RoomId,
        bool bIsParty,
        FOnUpsertRoomAdvertisementComplete CompletionDelegate,
        TWeakPtr<IOnlineSession> OnlineSessionWk,
        FName ExpectedSessionName);

    void OnSessionInviteReceived(
        const FUniqueNetId &UserId,
        const FUniqueNetId &FromId,
        const FString &AppId,
        const FOnlineSessionSearchResult &InviteResult,
        FRuntimePlatformServiceCallContextRef CallContext);

    void OnSessionInviteAccepted(
        const bool bWasSuccessful,
        const int32 ControllerId,
        FUniqueNetIdPtr UserId,
        const FOnlineSessionSearchResult &InviteResult,
        FRuntimePlatformServiceCallContextRef CallContext);

    void OnPresenceReceived(
        const FUniqueNetId &UserId,
        const TSharedRef<FOnlineUserPresence> &Presence,
        FRuntimePlatformServiceCallContextRef CallContext);

    void OnConnectionStatusChanged(
        const FString &ServiceName,
        EOnlineServerConnectionStatus::Type LastConnectionState,
        EOnlineServerConnectionStatus::Type ConnectionState,
        FRuntimePlatformServiceCallContextRef CallContext);

    static void OnPresenceRestoredAfterConnectivityRestored(
        FError ErrorCode,
        int32 LocalUserNum,
        FName SubsystemNameCaptured);

    static void OnRoomAdvertisementRestoredAfterConnectivityRestored(
        FError ErrorCode,
        int32 LocalUserNum,
        FName SubsystemNameCaptured);

    static void OnRoomAdvertisementClearedAfterConnectivityRestored(
        FError ErrorCode,
        int32 LocalUserNum,
        FName SubsystemNameCaptured);
};

};

namespace Redpoint::EOS::Platform::Integration::OSSv1::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1158129887,
    Redpoint::EOS::Platform::Integration::OSSv1::Services,
    FOSSv1RuntimePlatformPresenceService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()