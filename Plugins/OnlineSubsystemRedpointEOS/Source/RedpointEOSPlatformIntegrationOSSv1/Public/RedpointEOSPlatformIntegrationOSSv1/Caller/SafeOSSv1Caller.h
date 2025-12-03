// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineAvatarInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1644138735, Redpoint::EOS::Platform::Integration::OSSv1::Caller)
{
using namespace ::Redpoint::EOS::Async;

class REDPOINTEOSPLATFORMINTEGRATIONOSSV1_API FSafeOSSv1Caller
{
public:
    static void CreateSession(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FUniqueNetId &LocalUserId,
        const FName &SessionName,
        const FOnlineSessionSettings &SessionSettings,
        const FOnCreateSessionCompleteDelegate &OnComplete);

    [[nodiscard]] static TTask<bool> CreateSessionAsync(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FUniqueNetId &LocalUserId,
        const FName &SessionName,
        const FOnlineSessionSettings &SessionSettings);

private:
    static void OnCreateSessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        TWeakPtr<IOnlineSession> SessionInterface,
        FName RequestedSessionName,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnCreateSessionCompleteDelegate OnComplete);

public:
    static void JoinSession(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FUniqueNetId &LocalUserId,
        const FName &SessionName,
        const FOnlineSessionSearchResult &SearchResult,
        const FOnJoinSessionCompleteDelegate &OnComplete);

    [[nodiscard]] static TTask<EOnJoinSessionCompleteResult::Type> JoinSessionAsync(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FUniqueNetId &LocalUserId,
        const FName &SessionName,
        const FOnlineSessionSearchResult &SearchResult);

private:
    static void OnJoinSessionComplete(
        FName SessionName,
        EOnJoinSessionCompleteResult::Type JoinResult,
        TWeakPtr<IOnlineSession> SessionInterface,
        FName RequestedSessionName,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnJoinSessionCompleteDelegate OnComplete);

public:
    static void FindSessions(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FUniqueNetId &LocalUserId,
        const TSharedRef<FOnlineSessionSearch> &Search,
        const FOnFindSessionsCompleteDelegate &OnComplete);

private:
    static void OnFindSessionsComplete(
        bool bWasSuccessful,
        TWeakPtr<IOnlineSession> SessionInterface,
        TSharedRef<FOnlineSessionSearch> RequestedSearch,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnFindSessionsCompleteDelegate OnComplete);

public:
    static void UpdateSession(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FName &SessionName,
        FOnlineSessionSettings &SessionSettings,
        const FOnUpdateSessionCompleteDelegate &OnComplete);

    [[nodiscard]] static TTask<bool> UpdateSessionAsync(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FName &SessionName,
        FOnlineSessionSettings &SessionSettings);

private:
    static void OnUpdateSessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        TWeakPtr<IOnlineSession> SessionInterface,
        FName RequestedSessionName,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnUpdateSessionCompleteDelegate OnComplete);

public:
    static void DestroySession(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FName &SessionName,
        const FOnDestroySessionCompleteDelegate &OnComplete);

    [[nodiscard]] static TTask<bool> DestroySessionAsync(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FName &SessionName);

private:
    static void OnDestroySessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        TWeakPtr<IOnlineSession> SessionInterface,
        FName RequestedSessionName,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnDestroySessionCompleteDelegate OnComplete);

public:
    [[nodiscard]] static bool SendSessionInviteToFriend(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FUniqueNetId &LocalUserId,
        FName SessionName,
        const FUniqueNetId &Friend);

    static void GetAvatar(
        const TSharedRef<IOnlineAvatar> &AvatarInterface,
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        TSoftObjectPtr<UTexture> DefaultTexture,
        FOnGetAvatarComplete OnComplete);

private:
    static void OnGetAvatarComplete(
        bool bWasSuccessful,
        TSoftObjectPtr<UTexture> ResultTexture,
        TWeakPtr<IOnlineAvatar> AvatarInterface,
        TSharedRef<bool> EventFired,
        FOnGetAvatarComplete OnComplete);

public:
    static void GetAvatarUrl(
        const TSharedRef<IOnlineAvatar> &AvatarInterface,
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        FString DefaultAvatarUrl,
        FOnGetAvatarUrlComplete OnComplete);

private:
    static void OnGetAvatarUrlComplete(
        bool bWasSuccessful,
        FString ResultUrl,
        TWeakPtr<IOnlineAvatar> AvatarInterface,
        TSharedRef<bool> EventFired,
        FOnGetAvatarUrlComplete OnComplete);

public:
    static void GetFriendsList(
        const TSharedRef<IOnlineFriends> &FriendsInterface,
        int32 LocalUserNum,
        const FString &ListName,
        TArray<TSharedRef<FOnlineFriend>> &OutFriends);

    static void GetRecentPlayers(
        const TSharedRef<IOnlineFriends> &FriendsInterface,
        const FUniqueNetId &UserId,
        const FString &Namespace,
        TArray<TSharedRef<FOnlineRecentPlayer>> &OutRecentPlayers);

    static void GetBlockedPlayers(
        const TSharedRef<IOnlineFriends> &FriendsInterface,
        const FUniqueNetId &UserId,
        TArray<TSharedRef<FOnlineBlockedPlayer>> &OutBlockedPlayers);

public:
    static void ReadFriendsList(
        const TSharedRef<IOnlineFriends> &FriendsInterface,
        int32 LocalUserNum,
        const FString &ListName,
        const FOnReadFriendsListComplete &OnComplete);

private:
    static void OnReadFriendsListComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FString &ListName,
        const FString &ErrorStr,
        TWeakPtr<IOnlineFriends> FriendsInterface,
        int32 RequestedLocalUserNum,
        FString RequestedListName,
        TSharedRef<bool> EventFired,
        FOnReadFriendsListComplete OnComplete);

public:
    static void QueryBlockedPlayers(
        const TSharedRef<IOnlineFriends> &FriendsInterface,
        const FUniqueNetId &LocalUserId,
        const FOnQueryBlockedPlayersCompleteDelegate &OnComplete);

private:
    static void OnQueryBlockedPlayersComplete(
        const FUniqueNetId &UserId,
        bool bWasSuccessful,
        const FString &Error,
        TWeakPtr<IOnlineFriends> FriendsInterface,
        FString RequestedLocalUserIdString,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnQueryBlockedPlayersCompleteDelegate OnComplete);

public:
    static void QueryRecentPlayers(
        const TSharedRef<IOnlineFriends> &FriendsInterface,
        const FUniqueNetId &LocalUserId,
        const FString &Namespace,
        const FOnQueryRecentPlayersCompleteDelegate &OnComplete);

private:
    static void OnQueryRecentPlayersComplete(
        const FUniqueNetId &UserId,
        const FString &Namespace,
        bool bWasSuccessful,
        const FString &Error,
        TWeakPtr<IOnlineFriends> FriendsInterface,
        FString RequestedLocalUserIdString,
        FString RequestedNamespace,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnQueryRecentPlayersCompleteDelegate OnComplete);

public:
    [[nodiscard]] static bool ShowInviteUI(
        const TSharedRef<IOnlineExternalUI> &ExternalUIInterface,
        int32 LocalUserNum,
        const FName &SessionName);

    static void ShowLoginUI(
        const TSharedRef<IOnlineExternalUI> &ExternalUIInterface,
        int ControllerIndex,
        bool bShowOnlineOnly,
        bool bShowSkipButton,
        const FOnLoginUIClosedDelegate &OnComplete);

private:
    static void OnLoginUIClosed(
        TSharedPtr<const FUniqueNetId> UniqueId,
        int ControllerIndex,
        const FOnlineError &Error,
        TWeakPtr<IOnlineExternalUI> ExternalUIInterface,
        TSharedRef<bool> EventFired,
        FOnLoginUIClosedDelegate OnComplete);

public:
    static void Login(
        const TSharedRef<IOnlineIdentity> &IdentityInterface,
        int LocalUserNum,
        const FOnlineAccountCredentials &Credentials,
        const FOnLoginCompleteDelegate &OnComplete);

private:
    static void OnLoginComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &Error,
        TWeakPtr<IOnlineIdentity> IdentityInterface,
        int32 RequestedLocalUserNum,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnLoginCompleteDelegate OnComplete);

public:
    static void RegisterPlayers(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FName &SessionName,
        const TArray<FUniqueNetIdRef> &PlayerIds,
        const FOnRegisterPlayersCompleteDelegate &OnComplete);

    [[nodiscard]] static TTask<bool> RegisterPlayersAsync(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FName &SessionName,
        const TArray<FUniqueNetIdRef> &PlayerIds);

private:
    static void OnRegisterPlayersComplete(
        FName SessionName,
        const TArray<FUniqueNetIdRef> &PlayerIds,
        bool bWasSuccessful,
        TWeakPtr<IOnlineSession> SessionInterface,
        FName RequestedSessionName,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnRegisterPlayersCompleteDelegate OnComplete);

public:
    static void UnregisterPlayers(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FName &SessionName,
        const TArray<FUniqueNetIdRef> &PlayerIds,
        const FOnUnregisterPlayersCompleteDelegate &OnComplete);

    [[nodiscard]] static TTask<bool> UnregisterPlayersAsync(
        const TSharedRef<IOnlineSession> &SessionInterface,
        const FName &SessionName,
        const TArray<FUniqueNetIdRef> &PlayerIds);

private:
    static void OnUnregisterPlayersComplete(
        FName SessionName,
        const TArray<FUniqueNetIdRef> &PlayerIds,
        bool bWasSuccessful,
        TWeakPtr<IOnlineSession> SessionInterface,
        FName RequestedSessionName,
        TSharedRef<FDelegateHandle> DelegateHandle,
        FOnUnregisterPlayersCompleteDelegate OnComplete);
};

};

namespace Redpoint::EOS::Platform::Integration::OSSv1::Caller
{
REDPOINT_EOS_FILE_NS_EXPORT(1644138735, Redpoint::EOS::Platform::Integration::OSSv1::Caller, FSafeOSSv1Caller)
}

REDPOINT_EOS_CODE_GUARD_END()