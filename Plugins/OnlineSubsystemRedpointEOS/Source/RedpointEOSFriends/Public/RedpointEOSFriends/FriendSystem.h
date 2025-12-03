// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Async/Future.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSCore/Id/AccountIdMap.h"
#include "RedpointEOSCore/Utils/NamedTuple.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPlatform/Types/RelationshipType.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#include "RedpointEOSUserCache/UserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4025721429, Redpoint::EOS::Friends)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSFRIENDS_API IFriendSystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<IFriendSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IFriendSystem() = default;
    UE_NONCOPYABLE(IFriendSystem);
    virtual ~IFriendSystem() override = default;

    /**
     * The delegate type used for OnFriendsInitiallyLoaded.
     */
    typedef TMulticastDelegate<void(const FIdentityUserId &LocalUserId)> FOnFriendsInitiallyLoaded;

    /**
     * Fired when the friends, pending invites, recent players and blocked players have all been loaded for a local
     * player as part of login. While the user won't count as being signed in on IIdentitySystem, you can query for the
     * friends state.
     */
    [[nodiscard]] virtual FOnFriendsInitiallyLoaded &OnFriendsInitiallyLoaded() = 0;

    /**
     * The delegate type used for OnFriendsChanged.
     */
    typedef TMulticastDelegate<void(const FIdentityUserId &LocalUserId, ERelationshipType ChangedRelationships)>
        FOnFriendsChanged;

    /**
     * Fired when the friends, pending invites, recent players and/or blocked players have changed for a local user
     * after the initial login. The ChangedRelationships parameter is a combined flag of all the changed areas.
     */
    [[nodiscard]] virtual FOnFriendsChanged &OnFriendsChanged() = 0;

    /**
     * The delegate type used for OnFriendPresenceChanged.
     */
    typedef TMulticastDelegate<void(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &FriendUserId,
        const FPresenceUserState &NewFriendPresenceState)>
        FOnFriendPresenceChanged;

    /**
     * Fired when the presence state changes for a unified friend, or for a user for whom a pending inbound or outbound
     * friend request exists.
     */
    [[nodiscard]] virtual FOnFriendPresenceChanged &OnFriendPresenceChanged() = 0;

    /**
     * The delegate type used for OnNonUnifiedFriendPresenceChanged.
     */
    typedef TMulticastDelegate<void(
        const FIdentityUserId &LocalUserId,
        const FExternalUserInfoRef &NonUnifiedFriend,
        const EPresenceUserActivityState &NewFriendPresenceActivityState)>
        FOnNonUnifiedFriendPresenceChanged;

    /**
     * Fired when the presence state changes for a non-unified friend.
     */
    [[nodiscard]] virtual FOnNonUnifiedFriendPresenceChanged &OnNonUnifiedFriendPresenceChanged() = 0;

    /**
     * The delegate type used for OnFriendRequestReceived.
     */
    typedef TMulticastDelegate<void(const FIdentityUserId &LocalUserId, const FUserInfoRef &SendingRemoteUser)>
        FOnFriendRequestReceived;

    /**
     * Fired when a local user receives a friend request. The OnFriendsChanged event will also fire after this event.
     */
    [[nodiscard]] virtual FOnFriendRequestReceived &OnFriendRequestReceived() = 0;

    /**
     * Get a list of unified and non-unified friends for the local user.
     */
    [[nodiscard]] virtual FError GetFriends(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<FUserInfoRef> &OutUnifiedFriends,
        TArray<FExternalUserInfoRef> &OutNonUnifiedFriends) = 0;

    /**
     * Get the current presence state for a local user's unified friend.
     */
    [[nodiscard]] virtual TOptional<FPresenceUserState> GetUnifiedFriendPresence(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &FriendUserId) const = 0;

    /**
     * Get the current presence activity state for a local user's non-unified friend. Non-unified friends do not have
     * much presence information because they have never played the game before (by their nature, they don't have a
     * PUID). The only relevant presence information for a friend who has never played the game before is to know if
     * they're online to invite them to play the game for the first time.
     */
    [[nodiscard]] virtual TOptional<EPresenceUserActivityState> GetNonUnifiedFriendPresence(
        const FIdentityUserId &LocalUserId,
        const FExternalUserInfoRef &FriendUser) const = 0;

    /**
     * Get a list of inbound friend requests that the local user needs to respond to.
     */
    [[nodiscard]] virtual FError GetInboundFriendRequests(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<FUserInfoRef> &OutInboundFriendRequests) = 0;

    /**
     * Get a list of outbound friend requests that the local user is waiting on other users to respond to.
     */
    [[nodiscard]] virtual FError GetOutboundFriendRequests(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<FUserInfoRef> &OutOutboundFriendRequests) = 0;

    /**
     * The result type returned in a future from the SendFriendRequest operation.
     */
    REDPOINT_AGGREGATE(
        FSendFriendRequestResult,
        FError,
        ErrorCode,
        FIdentityUserId,
        LocalUserId,
        FIdentityUserId,
        RemoteUserId);

    /**
     * Send a friend request from the local user to the remote user.
     */
    [[nodiscard]] virtual TFuture<FSendFriendRequestResult> SendFriendRequest(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) = 0;

    /**
     * The result type returned in a future from the AcceptFriendRequest operation.
     */
    REDPOINT_AGGREGATE(
        FAcceptFriendRequestResult,
        FError,
        ErrorCode,
        FIdentityUserId,
        LocalUserId,
        FIdentityUserId,
        RemoteRequesterUserId);

    /**
     * Accept a friend request sent to the local user from the remote requester user.
     */
    [[nodiscard]] virtual TFuture<FAcceptFriendRequestResult> AcceptFriendRequest(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteRequesterUserId) = 0;

    /**
     * The result type returned in a future from the AcceptFriendRequest operation.
     */
    REDPOINT_AGGREGATE(
        FRejectFriendRequestResult,
        FError,
        ErrorCode,
        FIdentityUserId,
        LocalUserId,
        FIdentityUserId,
        RemoteRequesterUserId);

    /**
     * Reject a friend request sent to the local user from the remote requester user.
     */
    [[nodiscard]] virtual TFuture<FRejectFriendRequestResult> RejectFriendRequest(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteRequesterUserId) = 0;

    /**
     * Returns whether the specified friend is deletable. Friends are only deletable if they were added through the
     * cross-platform friends system; friends provided by runtime platform integrations can not be deleted.
     */
    [[nodiscard]] virtual bool IsFriendDeletable(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteFriendUserId) = 0;

    /**
     * The result type returned in a future from the DeleteFriend operation.
     */
    REDPOINT_AGGREGATE(
        FDeleteFriendResult,
        FError,
        ErrorCode,
        FIdentityUserId,
        LocalUserId,
        FIdentityUserId,
        RemoteFriendUserId);

    /**
     * Delete the cross-platform friend from the local user's friends list.
     */
    [[nodiscard]] virtual TFuture<FDeleteFriendResult> DeleteFriend(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteFriendUserId) = 0;

    /**
     * Get the list of recent players the local user has played with on multiplayer servers.
     */
    [[nodiscard]] virtual FError GetRecentPlayers(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<TTuple<FUserInfoRef, FDateTime>> &OutRecentPlayers) = 0;

    /**
     * Record a remote user as a recent player. This user will be recorded as a recent player for all local users
     * currently signed in (as all local users are expected to be playing on multiplayer servers).
     */
    virtual void RecordRecentPlayer(const FIdentityUserId &RemoteUserId) = 0;

    /**
     * Get the list of players that the local user has blocked.
     */
    [[nodiscard]] virtual FError GetBlockedPlayers(
        const FIdentityUserId &LocalUserId,
        TAccountIdMap<FUserInfoPtr> &OutBlockedPlayers) = 0;

    /**
     * The result type returned in a future from the DeleteFriend operation.
     */
    REDPOINT_AGGREGATE(
        FBlockPlayerResult,
        FError,
        ErrorCode,
        FIdentityUserId,
        LocalUserId,
        FIdentityUserId,
        RemoteUserId);

    /**
     * Add the remote user to the local user's block list.
     */
    [[nodiscard]] virtual TFuture<FBlockPlayerResult> BlockPlayer(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) = 0;

    /**
     * The result type returned in a future from the UnblockPlayer operation.
     */
    REDPOINT_AGGREGATE(
        FUnblockPlayerResult,
        FError,
        ErrorCode,
        FIdentityUserId,
        LocalUserId,
        FIdentityUserId,
        RemoteUserId);

    /**
     * Remove the remote user from the local user's block list.
     */
    [[nodiscard]] virtual TFuture<FUnblockPlayerResult> UnblockPlayer(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) = 0;

    /**
     * The result type returned in a future from the SetFriendAlias operation.
     */
    REDPOINT_AGGREGATE(
        FSetFriendAliasResult,
        FError,
        ErrorCode,
        FIdentityUserId,
        LocalUserId,
        FIdentityUserId,
        RemoteUserId);

    /**
     * Set an alias for the friend, which can be displayed instead of their display name.
     */
    [[nodiscard]] virtual TFuture<FSetFriendAliasResult> SetFriendAlias(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId,
        const FString &FriendAlias) = 0;

    /**
     * The result type returned in a future from the DeleteFriendAlias operation.
     */
    REDPOINT_AGGREGATE(
        FDeleteFriendAliasResult,
        FError,
        ErrorCode,
        FIdentityUserId,
        LocalUserId,
        FIdentityUserId,
        RemoteUserId);

    /**
     * Remove a previously set alias for a friend.
     */
    [[nodiscard]] virtual TFuture<FDeleteFriendAliasResult> DeleteFriendAlias(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) = 0;

    /**
     * Get the alias for a friend, if it is set.
     */
    [[nodiscard]] virtual TOptional<FString> GetFriendAlias(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &RemoteUserId) = 0;
};

}

namespace Redpoint::EOS::Friends
{
REDPOINT_EOS_FILE_NS_EXPORT(4025721429, Redpoint::EOS::Friends, IFriendSystem)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()