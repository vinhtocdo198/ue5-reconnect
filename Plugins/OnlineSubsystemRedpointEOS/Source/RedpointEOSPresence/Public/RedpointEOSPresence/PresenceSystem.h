// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceInterestChange.h"
#include "RedpointEOSPresence/PresenceMessageConnection.h"
#include "RedpointEOSPresence/PresenceUserState.h"
REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()
#include "RedpointEOSUserCache/UserInfo.h"
REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END()
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2615688832, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::UserCache;

class REDPOINTEOSPRESENCE_API IPresenceSystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<IPresenceSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IPresenceSystem() = default;
    UE_NONCOPYABLE(IPresenceSystem);
    virtual ~IPresenceSystem() override = default;

    /**
     * The delegate type used to signal presence state changes for remote users that we're interested in.
     */
    typedef TMulticastDelegate<void(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &TargetUserId,
        const FPresenceUserState &NewPresenceState)>
        FOnRemoteUserPresenceStateUpdated;

    /**
     * An event which fires whenever a remote user that we previously signalled interest in (via UpdatePresenceInterest)
     * has their presence information changed.
     *
     * The "InterestedSystem" parameter is used to isolate interest lists from different areas of code. Events will only
     * fired for an interested system if that system has indicated the target user is of interest via
     * UpdatePresenceInterest. The special value 'NAME_All' can be used to listen for presence change events across all
     * target users of interest, regardless of the calling code that marked them as of interest.
     */
    virtual FOnRemoteUserPresenceStateUpdated &OnRemoteUserPresenceStateUpdated(const FName &InterestedSystem) = 0;

    /**
     * The delegate type used to signal completion of the UpdatePresenceInterest operation.
     */
    typedef TDelegate<
        void(const FError &ErrorCode, const TMap<FIdentityUserId, FPresenceUserState> &RemoteUserPresenceState)>
        FOnUpdatePresenceInterestComplete;

    /**
     * Update the "interested" state for remote target users for the local user.
     *
     * The "InterestedSystem" parameter is used to isolate interest lists from different areas of code. For example,
     * this allows a "friends system" and custom game code to have two independent "interest users" lists and to receive
     * presence events only for the users they expect. The presence system will internally update presence state for
     * target users as long as at least one system indicates they are of interest. The special value 'NAME_All' can not
     * be passed to UpdatePresenceInterest.
     *
     * If bWaitForInitialPresenceData is set to true, CompletionDelegate won't fire until we've received presence
     * data from the remote user interest entries with bInterested set to true (which may be instant if we already have
     * presence cached for those remote users).
     *
     * If bWaitForInitialPresenceData is false, CompletionDelegate fires immediately and the presence information is
     * obtained in the background. In this case, the RemoteUserPresenceState parameter of the callback will always be
     * an empty map (even if we have cached information for some of the remote users).
     *
     * This function does not remove a target user from the interested list unless TargetUserInterest contains an
     * entry with that target user's ID and bInterested set to false.
     */
    virtual void UpdatePresenceInterest(
        const FName &InterestedSystem,
        const FIdentityUserId &LocalUserId,
        const TArray<FPresenceInterestChange> &TargetUserInterest,
        bool bWaitForInitialPresenceData,
        const FOnUpdatePresenceInterestComplete &CompletionDelegate) = 0;

    /**
     * Retrieve the latest known presence information for the target user, if available. There is no asynchronous
     * version of this API, because it always pulls from the cache, and the cache is always kept up-to-date based on the
     * interested user list set with UpdatePresenceInterest.
     *
     * This function can also be used to obtain the current presence state of the local user, for performing partial
     * updates with SetPresence.
     *
     * This function can return cached presence state for a target user as long as any interested system used
     * UpdatePresenceInterest to include them in the interest list. Or to put it another way, this function can return
     * presence information for target users your code did not declare an interest in with UpdatePresenceInterest, as
     * unrelated code may have declared them also to be of interest.
     */
    virtual TOptional<FPresenceUserState> GetPresence(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &TargetUserId) const = 0;

    /**
     * The delegate type used to signal completion of the SetPresence operation.
     */
    typedef TDelegate<void(const FError &ErrorCode)> FOnSetPresenceComplete;

    /**
     * Update the local user's presence state to NewPresenceState. The following fields are ignored by this function, as
     * they're managed by the presence system and can't be changed manually:
     *
     * - bPresenceAdvertised (controlled by runtime platform integrations to respect the local player's presence state)
     * - AdvertisedSessionId (determined from the room system)
     * - AdvertisedPartyId (determined from the room system)
     */
    virtual void SetPresence(
        const FIdentityUserId &LocalUserId,
        const FPresenceUserState &NewPresenceState,
        const FOnSetPresenceComplete &CompletionDelegate) = 0;

    /**
     * The delegate type used to signal completion of the TryOpenMessageConnection operation.
     */
    typedef TDelegate<void(const FError &ErrorCode, const TSharedPtr<IPresenceMessageConnection> &MessageConnection)>
        FOnTryOpenMessageConnection;

    /**
     * Attempt to open a presence messaging connection to the target user. This provides a low-frequency reliable
     * message transport from the local EOS user to another EOS user over EOS lobbies. Connections are unique to the
     * interested system.
     *
     * Internally the plugin uses this to send friend request related operations for cross-platform friends.
     */
    virtual void TryOpenMessageConnection(
        const FIdentityUserId &LocalUserId,
        const FIdentityUserId &TargetUserId,
        const FName &InterestedSystem,
        const FOnTryOpenMessageConnection &CompletionDelegate) = 0;

    /**
     * The delegate type used to signal when a new presence messaging connection is established with a local user.
     */
    typedef TMulticastDelegate<void(const TSharedRef<IPresenceMessageConnection> &MessageConnection)>
        FOnInboundMessageConnectionEstablished;

    /**
     * An event which fires when another EOS user establishes a presence messaging connection with us by calling
     * TryOpenMessageConnection.
     */
    virtual FOnInboundMessageConnectionEstablished &OnInboundMessageConnectionEstablished(
        const FName &InterestedSystem) = 0;

    /**
     * Returns the friend code assigned to the local user who is signed into the identity system. If the specified user
     * isn't signed in, the returned value is not set.
     */
    virtual TOptional<FString> GetFriendCode(const FIdentityUserId &LocalUserId) = 0;

    /**
     * The delegate type used to signal completion of the FindByFriendCode operation.
     */
    typedef TDelegate<void(const FError &ErrorCode, const TArray<FUserInfoRef> &DiscoveredUsers)>
        FOnFindByFriendCodeComplete;

    /**
     * Find another user by their friend code (returned by GetFriendCode). This operation is asynchronous and may return
     * multiple users. Friend codes have the following semantics:
     *
     * - Friend codes not guaranteed to be unique. It's extremely unlikely for a duplicate to occur, but still possible.
     * - Friend codes are generated every time a user signs in; they do not persist across game launches.
     * - Users are only discoverable by friend code if the target user is online at the time FindByFriendCode is called.
     * You can not discover users who are offline.
     */
    virtual void FindByFriendCode(
        const FIdentityUserId &LocalUserId,
        const FString &FriendCode,
        const FOnFindByFriendCodeComplete &CompletionDelegate) = 0;

    /**
     * An enumeration that represents the Internet connectivity status.
     */
    enum class EInternetConnectivityStatus
    {
        // The local machine can reach the Internet.
        Connected,

        // The local machine is unable to reach the Internet (or at the very least, the EOS backend).
        Disconnected,
    };

    /**
     * The delegate type used to signal when Internet connectivity is lost or restored.
     */
    typedef TMulticastDelegate<void(EInternetConnectivityStatus InternetConnectivityStatus)>
        FOnInternetConnectivityChanged;

    /**
     * An event which fires when Internet connectivity is lost or restored. The Internet connection is considered lost
     * when at least one local user disconnects from their cross-platform presence lobby with reason
     * ERoomLeaveReason::Disconnected. The Internet connection is considered restored when all local users have
     * cross-platform presence lobbies again. Failure to create the cross-platform presence lobby during login is not
     * considered an Internet connectivity loss.
     */
    virtual FOnInternetConnectivityChanged &OnInternetConnectivityChanged() = 0;

    /**
     * Wait until the local user's desired presence has been propagated out to Epic Online Services and the runtime
     * platform integrations. It is necessary to wait for presence to be reconciled prior to calling
     * SendRoomInviteToNonUnifiedFriend on runtime platform integrations, since otherwise you may be attempting to send
     * an invite to a room that the runtime platform integration isn't yet advertising.
     */
    virtual void WaitForReconciledPresence(
        const FIdentityUserId &LocalUserId,
        const FSimpleDelegate &CompletionDelegate) = 0;
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(2615688832, Redpoint::EOS::Presence, IPresenceSystem)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()