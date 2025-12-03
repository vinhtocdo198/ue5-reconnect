// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "OVR_Platform.h"
#include "OVR_User.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

#define REDPOINT_EOS_META_HAS_USER_GET_DISPLAY_NAME                                                                    \
    (PLATFORM_MAJOR_VERSION > 1 || (PLATFORM_MAJOR_VERSION == 1 && PLATFORM_MINOR_VERSION >= 51))
#define REDPOINT_EOS_META_HAS_USER_GET_BLOCKED_USERS                                                                   \
    (PLATFORM_MAJOR_VERSION > 1 || (PLATFORM_MAJOR_VERSION == 1 && PLATFORM_MINOR_VERSION >= 72))
// @note: This is minor version 81 because we need ovr_GroupPresenceOptions_SetDeeplinkMessageOverride present.
#define REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SET                                                                       \
    (PLATFORM_MAJOR_VERSION > 1 || (PLATFORM_MAJOR_VERSION == 1 && PLATFORM_MINOR_VERSION >= 81))
#define REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SEND_INVITES                                                              \
    (PLATFORM_MAJOR_VERSION > 1 || (PLATFORM_MAJOR_VERSION == 1 && PLATFORM_MINOR_VERSION >= 72))
#define REDPOINT_EOS_META_HAS_GROUP_PRESENCE_LAUNCH_INVITE_PANEL                                                       \
    (PLATFORM_MAJOR_VERSION > 1 || (PLATFORM_MAJOR_VERSION == 1 && PLATFORM_MINOR_VERSION >= 72))
#define REDPOINT_EOS_META_HAS_GROUP_PRESENCE_JOIN_INTENT_RECEIVED                                                      \
    (PLATFORM_MAJOR_VERSION > 1 || (PLATFORM_MAJOR_VERSION == 1 && PLATFORM_MINOR_VERSION >= 63))

namespace REDPOINT_EOS_FILE_NS_ID(1841280286, Redpoint::EOS::Platform::Integration::Meta)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::API;

class FMetaSDK
{
public:
    typedef TDelegate<void(const TOptional<ovrMessageHandle> &Message)> FOnResponseOrTimeout;

private:
    static bool bDispatcherRunning;
    static TMap<ovrRequest, TTuple<FOnResponseOrTimeout, FDateTime>> AwaitingRequests;
    static FCriticalSection DispatcherStartLock;

    static void AwaitRequest(const ovrRequest &Request, const FOnResponseOrTimeout &OnResponseOrTimeout);
    static bool TickDispatch(float);

public:
    FMetaSDK() = default;
    UE_NONCOPYABLE(FMetaSDK);
    ~FMetaSDK() = default;

private:
    static bool bIsInitAttempted;
    static bool bIsInitSuccessful;
    static FCriticalSection InitLock;

public:
    static bool InitIfNeeded();

#if PLATFORM_WINDOWS
    static bool IsMetaQuestLinkOnDesktop();
#endif

    typedef TMulticastDelegate<void(const FString &Deeplink)> FOnDeeplinkReceived;

private:
    static FOnDeeplinkReceived OnDeeplinkReceivedDelegate;

public:
    // @note: Deeplinks can come from application launch or group presence join intents. This event fires both.
    static FOnDeeplinkReceived &OnDeeplinkReceived();

    typedef TDelegate<void(const FError &ErrorCode, const TOptional<ovrUserHandle> &User)> FOnGetLoggedInUserComplete;
    static void GetLoggedInUser(const FOnGetLoggedInUserComplete &OnComplete);

    typedef TDelegate<void(const FError &ErrorCode, const TOptional<ovrUserHandle> &User)> FOnGetUserComplete;
    static void GetUser(ovrID UserId, const FOnGetUserComplete &OnComplete);

    typedef TDelegate<void(const FError &ErrorCode)> FOnGetIsViewerEntitledComplete;
    static void GetIsViewerEntitled(const FOnGetIsViewerEntitledComplete &OnComplete);

    typedef TDelegate<void(const FError &ErrorCode, const TOptional<FString> &Nonce)> FOnGetUserProofComplete;
    static void GetUserProof(const FOnGetUserProofComplete &OnComplete);

    typedef TDelegate<void(const FError &ErrorCode, const TOptional<TArray<ovrUserHandle>> &Friends)>
        FOnGetLoggedInUserFriendsComplete;
    static void GetLoggedInUserFriends(const FOnGetLoggedInUserFriendsComplete &OnComplete);

    typedef TDelegate<void(const FError &ErrorCode, const TOptional<TArray<ovrUserHandle>> &Users)>
        FOnGetFullUserArrayComplete;
    static void GetFullUserArray(
        ovrUserArrayHandle UserArray,
        TArray<ovrUserHandle> Accumulated,
        const FOnGetFullUserArrayComplete &OnComplete);

#if REDPOINT_EOS_META_HAS_USER_GET_BLOCKED_USERS
    typedef TDelegate<void(const FError &ErrorCode, const TOptional<TArray<ovrBlockedUserHandle>> &BlockedUsers)>
        FOnGetBlockedUsersComplete;
    static void GetBlockedUsers(const FOnGetBlockedUsersComplete &OnComplete);

    typedef TDelegate<void(const FError &ErrorCode, const TOptional<TArray<ovrBlockedUserHandle>> &BlockedUsers)>
        FOnGetFullBlockedUserArrayComplete;
    static void GetFullBlockedUserArray(
        ovrBlockedUserArrayHandle UserArray,
        TArray<ovrBlockedUserHandle> Accumulated,
        const FOnGetFullBlockedUserArrayComplete &OnComplete);
#endif

    static TOptional<FString> UserIdToString(ovrID UserId);

    static TOptional<ovrID> UserIdFromString(const FString &UserId);

#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SET
    typedef TDelegate<void(const FError &ErrorCode)> FOnSetGroupPresenceComplete;
    static void SetGroupPresence(ovrGroupPresenceOptionsHandle Options, const FOnSetGroupPresenceComplete &OnComplete);
#endif

#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SEND_INVITES
    typedef TDelegate<void(const FError &ErrorCode)> FOnSendGroupPresenceInviteComplete;
    static void SendGroupPresenceInvite(ovrID TargetUserId, const FOnSendGroupPresenceInviteComplete &OnComplete);
#endif

#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_LAUNCH_INVITE_PANEL
    typedef TDelegate<void(const FError &ErrorCode)> FOnLaunchGroupPresenceInvitePanelComplete;
    static void LaunchGroupPresenceInvitePanel(const FOnLaunchGroupPresenceInvitePanelComplete &OnComplete);
#endif
};

}

namespace Redpoint::EOS::Platform::Integration::Meta
{
REDPOINT_EOS_FILE_NS_EXPORT(1841280286, Redpoint::EOS::Platform::Integration::Meta, FMetaSDK)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()