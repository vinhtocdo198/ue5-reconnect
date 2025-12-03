// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/FlagEnum.h"
#include "RedpointEOSCore/Id/AccountIdMap.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSCore/Utils/PeriodicTimer.h"
#include "RedpointEOSFriends/Database/SerializedFriendDatabase.h"
#include "RedpointEOSFriends/FriendSystem.h"
#include "RedpointEOSFriends/LocalUserFriendState.h"
#include "RedpointEOSIdentity/IdentityUser.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(775074533, Redpoint::EOS::Friends)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Friends;
using namespace ::Redpoint::EOS::Friends::Database;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSFRIENDS_API FFriendSystemImpl : public IFriendSystem,
                                                 public IHasEventRegistration,
                                                 public TSharedFromThis<FFriendSystemImpl>
{
private:
    FPlatformHandle PlatformHandle;
    TSharedRef<FExclusiveAsyncOperationQueue> OperationQueue;
    TAccountIdMap<TSharedRef<FLocalUserFriendState>> LocalUserFriendStates;
    TSharedRef<FPeriodicTimer> SyncTimer;
    FDelegateHandle OnInboundMessageConnectionEstablishedDelegateHandle;
    FDelegateHandle OnRemoteUserPresenceStateUpdatedDelegateHandle;
    FOnFriendsInitiallyLoaded FriendsInitiallyLoadedDelegate;
    FOnFriendsChanged FriendsChangedDelegate;
    FOnFriendPresenceChanged FriendPresenceChangedDelegate;
    FOnNonUnifiedFriendPresenceChanged NonUnifiedFriendPresenceChangedDelegate;
    FOnFriendRequestReceived FriendRequestReceivedDelegate;

public:
    FFriendSystemImpl(const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FFriendSystemImpl);
    virtual ~FFriendSystemImpl() override = default;

    virtual void RegisterEvents() override;

    [[nodiscard]] TFuture<FError> AddLocalUser(const FIdentityUserRef &LocalUserId);
    [[nodiscard]] TFuture<void> RemoveLocalUser(const FIdentityUserRef &LocalUserId);

private:
    [[nodiscard]] TFuture<FError> ImmediateSave(const FIdentityUserId &LocalUserId);
    [[nodiscard]] TFuture<FError> ImmediateRefresh(
        const FIdentityUserId &LocalUserId,
        const TSharedRef<FLocalUserFriendState> &LocalUserFriendState,
        bool bSilenceEvents);

public:
    enum class EOperationType : uint8
    {
        None = 0x0,
        Save = 0x1,
        Refresh = 0x2,
    };

private:
    [[nodiscard]] TFuture<FError> Immediate(const FIdentityUserId &LocalUserId, EOperationType OperationType);
    [[nodiscard]] TFuture<FError> Queue(const FIdentityUserId &LocalUserId, EOperationType OperationType);

    static void AddPendingFriendRequest(
        const TSharedRef<FLocalUserFriendState> &State,
        const FIdentityUserId &RemoteUserId,
        FSerializedPendingFriendRequest &&Request,
        const FString &Context);

    static void RemovePendingFriendRequest(
        const TSharedRef<FLocalUserFriendState> &State,
        const FIdentityUserId &RemoteUserId,
        const FString &Context);

    void OnPresenceInboundMessageConnectionEstablished(const TSharedRef<IPresenceMessageConnection> &MessageConnection);

    void OnRemoteUserPresenceStateUpdated(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &TargetUserId,
        const FPresenceUserState &NewPresenceState);

    void OnPeriodicTimerQueuePendingOperations();

    void DeferPendingFriendRequestOperation(const FIdentityUserId &LocalUserId, const FIdentityUserId &RemoteUserId);

    [[nodiscard]] TFuture<FError> QueueSendFriendRequestAndWait(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId);

    [[nodiscard]] TFuture<FError> QueueSendFriendAcceptanceAndWait(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId);

    [[nodiscard]] TFuture<FError> QueueSendFriendRejectionAndWait(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId);

    [[nodiscard]] TFuture<FError> QueueSendFriendDeletionAndWait(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId);

    void UpdateNonUnifiedFriendPresence(
        int LocalUserNum,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId,
        EPresenceUserActivityState NewPresenceActivityState);

public:
    [[nodiscard]] virtual FOnFriendsInitiallyLoaded &OnFriendsInitiallyLoaded() override;

    [[nodiscard]] virtual FOnFriendsChanged &OnFriendsChanged() override;

    [[nodiscard]] virtual FOnFriendPresenceChanged &OnFriendPresenceChanged() override;

    [[nodiscard]] virtual FOnNonUnifiedFriendPresenceChanged &OnNonUnifiedFriendPresenceChanged() override;

    [[nodiscard]] virtual FOnFriendRequestReceived &OnFriendRequestReceived() override;

    [[nodiscard]] virtual FError GetFriends(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<FUserInfoRef> &OutUnifiedFriends,
        TArray<FExternalUserInfoRef> &OutNonUnifiedFriends) override;

    [[nodiscard]] virtual TOptional<FPresenceUserState> GetUnifiedFriendPresence(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &FriendUserId) const override;

    [[nodiscard]] virtual TOptional<EPresenceUserActivityState> GetNonUnifiedFriendPresence(
        const FIdentityUserId &LocalUserId,
        const FExternalUserInfoRef &FriendUser) const override;

    [[nodiscard]] virtual FError GetInboundFriendRequests(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<FUserInfoRef> &OutInboundFriendRequests) override;

    [[nodiscard]] virtual FError GetOutboundFriendRequests(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<FUserInfoRef> &OutOutboundFriendRequests) override;

    [[nodiscard]] virtual TFuture<FSendFriendRequestResult> SendFriendRequest(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) override;

    [[nodiscard]] virtual TFuture<FAcceptFriendRequestResult> AcceptFriendRequest(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteRequesterUserId) override;

    [[nodiscard]] virtual TFuture<FRejectFriendRequestResult> RejectFriendRequest(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteRequesterUserId) override;

    [[nodiscard]] virtual bool IsFriendDeletable(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteFriendUserId) override;

    [[nodiscard]] virtual TFuture<FDeleteFriendResult> DeleteFriend(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteFriendUserId) override;

    [[nodiscard]] virtual FError GetRecentPlayers(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<TTuple<FUserInfoRef, FDateTime>> &OutRecentPlayers) override;

    virtual void RecordRecentPlayer(const FIdentityUserId &InAccountId) override;

    [[nodiscard]] virtual FError GetBlockedPlayers(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<FUserInfoPtr> &OutBlockedPlayers) override;

    [[nodiscard]] virtual TFuture<FBlockPlayerResult> BlockPlayer(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) override;

    [[nodiscard]] virtual TFuture<FUnblockPlayerResult> UnblockPlayer(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) override;

    [[nodiscard]] virtual TFuture<FSetFriendAliasResult> SetFriendAlias(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId,
        const FString &FriendAlias) override;

    [[nodiscard]] virtual TFuture<FDeleteFriendAliasResult> DeleteFriendAlias(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) override;

    [[nodiscard]] virtual TOptional<FString> GetFriendAlias(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) override;
};

REDPOINT_EOS_DECLARE_FLAG_ENUMERATION_OPERATORS(FFriendSystemImpl::EOperationType)

}

namespace Redpoint::EOS::Friends
{
REDPOINT_EOS_FILE_NS_EXPORT(775074533, Redpoint::EOS::Friends, FFriendSystemImpl)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()