// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "IPAddress.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/UniqueNetIdEOSSession.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/Sessions/UpdateSession.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class FOnlineSubsystemEOS;
class FCleanShutdown;

namespace REDPOINT_EOS_FILE_NS_ID(809401287, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session)
{
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Sessions;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineSessionInterfaceEOS : public IOnlineSession,
                                                                  public IHasEventRegistration,
                                                                  public TSharedFromThis<FOnlineSessionInterfaceEOS>
{
    friend ::FOnlineSubsystemEOS;
    friend ::FCleanShutdown;

private:
    FPlatformHandle PlatformHandle;

    FOnlineIdentityInterfaceEOSRef Identity;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TSharedPtr<IOnlineFriends, ESPMode::ThreadSafe> Friends;
#endif
    TSharedPtr<IConfig> Config;

    /** Critical sections for thread safe operation of session lists */
    mutable FCriticalSection SessionLock;

    /** Current session settings */
    TArray<FNamedOnlineSession> Sessions;

    FString GetBucketId(const FOnlineSessionSettings &SessionSettings);
    EOS_EResult ApplyConnectionSettingsToModificationHandle(
        const FName &SessionName,
        const TSharedRef<const FInternetAddr> &InternetAddr,
        const TArray<TSharedRef<const FInternetAddr>> &DeveloperInternetAddrs,
        TSharedRef<TReleasable<EOS_HSessionModification>> Handle,
        const FOnlineSessionSettings &SessionSettings,
        bool &bIsPeerToPeerAddress);
    EOS_EResult ApplySettingsToModificationHandle(
        const FName &SessionName,
        const TSharedPtr<const FUniqueNetIdEOS> &HostingUserId,
        const FOnlineSessionSettings &SessionSettings,
        TSharedRef<TReleasable<EOS_HSessionModification>> Handle,
        const FOnlineSessionSettings *ExistingSessionSettings);

    void MetricsSend_BeginPlayerSession(
        const FUniqueNetIdEOS &UserId,
        const FString &SessionId,
        const FString &GameServerAddress);
    void MetricsSend_EndPlayerSession(const FUniqueNetIdEOS &UserId);

    void Handle_SessionAddressChanged(
        const UE::Online::FAccountId &ListeningUserId,
        const TSharedPtr<const FInternetAddr> &InternetAddr,
        const TArray<TSharedRef<const FInternetAddr>> &DeveloperInternetAddrs);

public:
    virtual void RegisterEvents() override;

private:
    TSharedPtr<EOSEventHandle<EOS_Sessions_JoinSessionAcceptedCallbackInfo>> Unregister_JoinSessionAccepted;
    TSharedPtr<EOSEventHandle<EOS_Sessions_SessionInviteAcceptedCallbackInfo>> Unregister_SessionInviteAccepted;
    TSharedPtr<EOSEventHandle<EOS_Sessions_SessionInviteReceivedCallbackInfo>> Unregister_SessionInviteReceived;

    void Handle_JoinSessionAccepted(const EOS_Sessions_JoinSessionAcceptedCallbackInfo *Data);
    void Handle_SessionInviteAccepted(const EOS_Sessions_SessionInviteAcceptedCallbackInfo *Data);
    void Handle_SessionInviteReceived(const EOS_Sessions_SessionInviteReceivedCallbackInfo *Data);

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    bool ShouldHaveSyntheticSession(const FOnlineSessionSettings &SessionSettings) const;
#endif

    TTask<void> FinalizeForCreateSessionInternal(
        FError Error,
        TSharedRef<FUniqueNetIdEOSSession> SessionId,
        FName SessionName,
        TSharedRef<const FUniqueNetIdEOS> EOSHostingUser);
    void OnUpdateSessionCompleteForCreateSessionInternal(
        const FUpdateSession::Result &Result,
        FName SessionName,
        TSharedRef<const FUniqueNetIdEOS> EOSHostingUser,
        int32 SyntheticMaxPlayers);
    bool CreateSessionInternal(
        const FUniqueNetId &HostingPlayerId,
        FName SessionName,
        const FOnlineSessionSettings &NewSessionSettings);

    void OnUpdateSessionCompleteForUpdateSessionInternal(const FUpdateSession::Result &Result, FName SessionName);
    bool UpdateSessionInternal(
        FName SessionName,
        FOnlineSessionSettings &UpdatedSessionSettings,
        bool bShouldRefreshOnlineData);

public:
    FOnlineSessionInterfaceEOS(
        const FPlatformHandle &InPlatformHandle,
        const FOnlineIdentityInterfaceEOSRef &InIdentity,
        const TSharedPtr<IOnlineFriends, ESPMode::ThreadSafe> &InFriends,
        const TSharedRef<IConfig> &InConfig);
    UE_NONCOPYABLE(FOnlineSessionInterfaceEOS);

    virtual FNamedOnlineSession *AddNamedSession(FName SessionName, const FOnlineSessionSettings &SessionSettings)
        override;
    virtual FNamedOnlineSession *AddNamedSession(FName SessionName, const FOnlineSession &Session) override;

    virtual TSharedPtr<const FUniqueNetId> CreateSessionIdFromString(const FString &SessionIdStr) override;
    virtual FNamedOnlineSession *GetNamedSession(FName SessionName) override;
    virtual void RemoveNamedSession(FName SessionName) override;
    virtual bool HasPresenceSession() override;
    virtual EOnlineSessionState::Type GetSessionState(FName SessionName) const override;
    virtual bool CreateSession(
        int32 HostingPlayerNum,
        FName SessionName,
        const FOnlineSessionSettings &NewSessionSettings) override;
    virtual bool CreateSession(
        const FUniqueNetId &HostingPlayerId,
        FName SessionName,
        const FOnlineSessionSettings &NewSessionSettings) override;
    virtual bool StartSession(FName SessionName) override;
    virtual bool UpdateSession(
        FName SessionName,
        FOnlineSessionSettings &UpdatedSessionSettings,
        bool bShouldRefreshOnlineData = true) override;
    virtual bool EndSession(FName SessionName) override;
    virtual bool DestroySession(
        FName SessionName,
        const FOnDestroySessionCompleteDelegate &CompletionDelegate = FOnDestroySessionCompleteDelegate()) override;
    virtual bool IsPlayerInSession(FName SessionName, const FUniqueNetId &UniqueId) override;
    virtual bool StartMatchmaking(
        const TArray<TSharedRef<const FUniqueNetId>> &LocalPlayers,
        FName SessionName,
        const FOnlineSessionSettings &NewSessionSettings,
        TSharedRef<FOnlineSessionSearch> &SearchSettings) override;
    virtual bool CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName) override;
    virtual bool CancelMatchmaking(const FUniqueNetId &SearchingPlayerId, FName SessionName) override;
    virtual bool FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch> &SearchSettings)
        override;
    virtual bool FindSessions(
        const FUniqueNetId &SearchingPlayerId,
        const TSharedRef<FOnlineSessionSearch> &SearchSettings) override;
    virtual bool FindSessionById(
        const FUniqueNetId &SearchingUserId,
        const FUniqueNetId &SessionId,
        const FUniqueNetId &FriendId,
        const FOnSingleSessionResultCompleteDelegate &CompletionDelegate) override;
    virtual bool CancelFindSessions() override;
    virtual bool PingSearchResults(const FOnlineSessionSearchResult &SearchResult) override;
    virtual bool JoinSession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult &DesiredSession)
        override;
    virtual bool JoinSession(
        const FUniqueNetId &LocalUserId,
        FName SessionName,
        const FOnlineSessionSearchResult &DesiredSession) override;
    virtual bool JoinSession(
        const FUniqueNetId &LocalUserId,
        FName SessionName,
        const FOnlineSessionSearchResult &DesiredSession,
        const FOnJoinSessionCompleteDelegate &OnComplete);
    virtual bool FindFriendSession(int32 LocalUserNum, const FUniqueNetId &Friend) override;
    virtual bool FindFriendSession(const FUniqueNetId &LocalUserId, const FUniqueNetId &Friend) override;
    virtual bool FindFriendSession(
        const FUniqueNetId &LocalUserId,
        const TArray<TSharedRef<const FUniqueNetId>> &FriendList) override;

private:
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    bool SendSessionInviteToRedpointEOSFriend(
        int32 LocalUserNum,
        TSharedRef<const FUniqueNetIdEOSSession> SessionId,
        TSharedRef<FOnlineFriend> FriendObj);
#endif

public:
    virtual bool SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId &Friend) override;
    virtual bool SendSessionInviteToFriend(
        const FUniqueNetId &LocalUserId,
        FName SessionName,
        const FUniqueNetId &Friend) override;
    virtual bool SendSessionInviteToFriends(
        int32 LocalUserNum,
        FName SessionName,
        const TArray<TSharedRef<const FUniqueNetId>> &Friends) override;
    virtual bool SendSessionInviteToFriends(
        const FUniqueNetId &LocalUserId,
        FName SessionName,
        const TArray<TSharedRef<const FUniqueNetId>> &Friends) override;
    virtual bool GetResolvedConnectString(FName SessionName, FString &ConnectInfo, FName PortType = NAME_GamePort)
        override;
    virtual bool GetResolvedConnectString(
        const FOnlineSessionSearchResult &SearchResult,
        FName PortType,
        FString &ConnectInfo) override;
    virtual FOnlineSessionSettings *GetSessionSettings(FName SessionName) override;
    virtual bool RegisterPlayer(FName SessionName, const FUniqueNetId &PlayerId, bool bWasInvited) override;
    virtual bool RegisterPlayers(
        FName SessionName,
        const TArray<TSharedRef<const FUniqueNetId>> &Players,
        bool bWasInvited = false) override;
    virtual bool UnregisterPlayer(FName SessionName, const FUniqueNetId &PlayerId) override;
    virtual bool UnregisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>> &Players) override;
    virtual void RegisterLocalPlayer(
        const FUniqueNetId &PlayerId,
        FName SessionName,
        const FOnRegisterLocalPlayerCompleteDelegate &Delegate) override;
    virtual void UnregisterLocalPlayer(
        const FUniqueNetId &PlayerId,
        FName SessionName,
        const FOnUnregisterLocalPlayerCompleteDelegate &Delegate) override;
    virtual int32 GetNumSessions() override;
    virtual void DumpSessionState() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(
    809401287,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session,
    FOnlineSessionInterfaceEOS)
}

REDPOINT_EOS_CODE_GUARD_END()