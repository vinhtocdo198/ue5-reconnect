// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/Caller/SafeOSSv1Caller.h"

#include "OnlineSessionSettings.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1644138735, Redpoint::EOS::Platform::Integration::OSSv1::Caller)
{
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;

void FSafeOSSv1Caller::CreateSession(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FUniqueNetId &LocalUserId,
    const FName &SessionName,
    const FOnlineSessionSettings &SessionSettings,
    const FOnCreateSessionCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateLambda([&bDidExecuteCallback](FName, bool) {
            bDidExecuteCallback = true;
        }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle =
        SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnCreateSessionComplete,
            TWeakPtr<IOnlineSession>(SessionInterface),
            SessionName,
            DelegateHandle,
            OnComplete));

    auto bStarted = SessionInterface->CreateSession(LocalUserId, SessionName, SessionSettings);
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(SessionName, false);
        }
    }
}

TTask<bool> FSafeOSSv1Caller::CreateSessionAsync(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FUniqueNetId &LocalUserId,
    const FName &SessionName,
    const FOnlineSessionSettings &SessionSettings)
{
    auto DeferredTask = TTask<bool>::Deferred();

    FSafeOSSv1Caller::CreateSession(
        SessionInterface,
        LocalUserId,
        SessionName,
        SessionSettings,
        FOnCreateSessionCompleteDelegate::CreateLambda([DeferredTask](FName, bool bWasSuccessful) {
            DeferredTask.SetValue(bWasSuccessful);
        }));

    return DeferredTask;
}

void FSafeOSSv1Caller::OnCreateSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    TWeakPtr<IOnlineSession> SessionInterface,
    FName RequestedSessionName,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnCreateSessionCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (!SessionName.IsEqual(RequestedSessionName))
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%s != %s): %s"),
            *SessionName.ToString(),
            *RequestedSessionName.ToString(),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto SessionInterfacePinned = SessionInterface.Pin();
    if (SessionInterfacePinned.IsValid())
    {
        SessionInterfacePinned->ClearOnCreateSessionCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(SessionName, bWasSuccessful);
}

void FSafeOSSv1Caller::JoinSession(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FUniqueNetId &LocalUserId,
    const FName &SessionName,
    const FOnlineSessionSearchResult &SearchResult,
    const FOnJoinSessionCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateLambda([&bDidExecuteCallback](FName, EOnJoinSessionCompleteResult::Type) {
            bDidExecuteCallback = true;
        }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle =
        SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnJoinSessionComplete,
            TWeakPtr<IOnlineSession>(SessionInterface),
            SessionName,
            DelegateHandle,
            OnComplete));

    auto bStarted = SessionInterface->JoinSession(LocalUserId, SessionName, SearchResult);
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(SessionName, EOnJoinSessionCompleteResult::UnknownError);
        }
    }
}

TTask<EOnJoinSessionCompleteResult::Type> FSafeOSSv1Caller::JoinSessionAsync(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FUniqueNetId &LocalUserId,
    const FName &SessionName,
    const FOnlineSessionSearchResult &SearchResult)
{
    auto DeferredTask = TTask<EOnJoinSessionCompleteResult::Type>::Deferred();

    FSafeOSSv1Caller::JoinSession(
        SessionInterface,
        LocalUserId,
        SessionName,
        SearchResult,
        FOnJoinSessionCompleteDelegate::CreateLambda([DeferredTask](FName, EOnJoinSessionCompleteResult::Type Result) {
            DeferredTask.SetValue(Result);
        }));

    return DeferredTask;
}

void FSafeOSSv1Caller::OnJoinSessionComplete(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type JoinResult,
    TWeakPtr<IOnlineSession> SessionInterface,
    FName RequestedSessionName,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnJoinSessionCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (!SessionName.IsEqual(RequestedSessionName))
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%s != %s): %s"),
            *SessionName.ToString(),
            *RequestedSessionName.ToString(),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto SessionInterfacePinned = SessionInterface.Pin();
    if (SessionInterfacePinned.IsValid())
    {
        SessionInterfacePinned->ClearOnJoinSessionCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(SessionName, JoinResult);
}

void FSafeOSSv1Caller::FindSessions(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FUniqueNetId &LocalUserId,
    const TSharedRef<FOnlineSessionSearch> &Search,
    const FOnFindSessionsCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateLambda([&bDidExecuteCallback](bool) {
            bDidExecuteCallback = true;
        }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle =
        SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnFindSessionsComplete,
            TWeakPtr<IOnlineSession>(SessionInterface),
            Search,
            DelegateHandle,
            OnComplete));

    auto bStarted = SessionInterface->FindSessions(LocalUserId, Search);
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(false);
        }
    }
}

void FSafeOSSv1Caller::OnFindSessionsComplete(
    bool bWasSuccessful,
    TWeakPtr<IOnlineSession> SessionInterface,
    TSharedRef<FOnlineSessionSearch> RequestedSearch,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnFindSessionsCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (RequestedSearch->SearchState != EOnlineAsyncTaskState::Done &&
        RequestedSearch->SearchState != EOnlineAsyncTaskState::Failed)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (SearchState != Done || Failed): %s"),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto SessionInterfacePinned = SessionInterface.Pin();
    if (SessionInterfacePinned.IsValid())
    {
        SessionInterfacePinned->ClearOnFindSessionsCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(bWasSuccessful);
}

void FSafeOSSv1Caller::UpdateSession(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FName &SessionName,
    FOnlineSessionSettings &SessionSettings,
    const FOnUpdateSessionCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(
        FOnUpdateSessionCompleteDelegate::CreateLambda([&bDidExecuteCallback](FName, bool) {
            bDidExecuteCallback = true;
        }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle =
        SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(FOnUpdateSessionCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnUpdateSessionComplete,
            TWeakPtr<IOnlineSession>(SessionInterface),
            SessionName,
            DelegateHandle,
            OnComplete));

    auto bStarted = SessionInterface->UpdateSession(SessionName, SessionSettings);
    SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(SessionName, false);
        }
    }
}

TTask<bool> FSafeOSSv1Caller::UpdateSessionAsync(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FName &SessionName,
    FOnlineSessionSettings &SessionSettings)
{
    auto DeferredTask = TTask<bool>::Deferred();

    FSafeOSSv1Caller::UpdateSession(
        SessionInterface,
        SessionName,
        SessionSettings,
        FOnUpdateSessionCompleteDelegate::CreateLambda([DeferredTask](FName, bool bWasSuccessful) {
            DeferredTask.SetValue(bWasSuccessful);
        }));

    return DeferredTask;
}

void FSafeOSSv1Caller::OnUpdateSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    TWeakPtr<IOnlineSession> SessionInterface,
    FName RequestedSessionName,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnUpdateSessionCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (!SessionName.IsEqual(RequestedSessionName))
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%s != %s): %s"),
            *SessionName.ToString(),
            *RequestedSessionName.ToString(),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto SessionInterfacePinned = SessionInterface.Pin();
    if (SessionInterfacePinned.IsValid())
    {
        SessionInterfacePinned->ClearOnUpdateSessionCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(SessionName, bWasSuccessful);
}

void FSafeOSSv1Caller::DestroySession(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FName &SessionName,
    const FOnDestroySessionCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateLambda([&bDidExecuteCallback](FName, bool) {
            bDidExecuteCallback = true;
        }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle =
        SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnDestroySessionComplete,
            TWeakPtr<IOnlineSession>(SessionInterface),
            SessionName,
            DelegateHandle,
            OnComplete));

    auto bStarted = SessionInterface->DestroySession(SessionName);
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(SessionName, false);
        }
    }
}

TTask<bool> FSafeOSSv1Caller::DestroySessionAsync(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FName &SessionName)
{
    auto DeferredTask = TTask<bool>::Deferred();

    FSafeOSSv1Caller::DestroySession(
        SessionInterface,
        SessionName,
        FOnDestroySessionCompleteDelegate::CreateLambda([DeferredTask](FName, bool bWasSuccessful) {
            DeferredTask.SetValue(bWasSuccessful);
        }));

    return DeferredTask;
}

void FSafeOSSv1Caller::OnDestroySessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    TWeakPtr<IOnlineSession> SessionInterface,
    FName RequestedSessionName,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnDestroySessionCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (!SessionName.IsEqual(RequestedSessionName))
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%s != %s): %s"),
            *SessionName.ToString(),
            *RequestedSessionName.ToString(),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto SessionInterfacePinned = SessionInterface.Pin();
    if (SessionInterfacePinned.IsValid())
    {
        SessionInterfacePinned->ClearOnDestroySessionCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(SessionName, bWasSuccessful);
}

bool FSafeOSSv1Caller::SendSessionInviteToFriend(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FUniqueNetId &LocalUserId,
    FName SessionName,
    const FUniqueNetId &Friend)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    return SessionInterface->SendSessionInviteToFriend(LocalUserId, SessionName, Friend);
}

void FSafeOSSv1Caller::GetAvatar(
    const TSharedRef<IOnlineAvatar> &AvatarInterface,
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId,
    TSoftObjectPtr<UTexture> DefaultTexture,
    FOnGetAvatarComplete OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    auto EventFired = MakeShared<bool>();
    if (!AvatarInterface->GetAvatar(
            LocalUserId,
            TargetUserId,
            DefaultTexture,
            FOnGetAvatarComplete::CreateStatic(
                &FSafeOSSv1Caller::OnGetAvatarComplete,
                TWeakPtr<IOnlineAvatar>(AvatarInterface),
                EventFired,
                OnComplete)))
    {
        if (!*EventFired)
        {
            *EventFired = true;
            OnComplete.ExecuteIfBound(false, DefaultTexture);
        }
    }
}

void FSafeOSSv1Caller::OnGetAvatarComplete(
    bool bWasSuccessful,
    TSoftObjectPtr<UTexture> ResultTexture,
    TWeakPtr<IOnlineAvatar> AvatarInterface,
    TSharedRef<bool> EventFired,
    FOnGetAvatarComplete OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (!*EventFired)
    {
        *EventFired = true;
        OnComplete.ExecuteIfBound(bWasSuccessful, ResultTexture);
    }
}

void FSafeOSSv1Caller::GetAvatarUrl(
    const TSharedRef<IOnlineAvatar> &AvatarInterface,
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId,
    FString DefaultAvatarUrl,
    FOnGetAvatarUrlComplete OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    auto EventFired = MakeShared<bool>();
    if (!AvatarInterface->GetAvatarUrl(
            LocalUserId,
            TargetUserId,
            DefaultAvatarUrl,
            FOnGetAvatarUrlComplete::CreateStatic(
                &FSafeOSSv1Caller::OnGetAvatarUrlComplete,
                TWeakPtr<IOnlineAvatar>(AvatarInterface),
                EventFired,
                OnComplete)))
    {
        if (!*EventFired)
        {
            *EventFired = true;
            OnComplete.ExecuteIfBound(false, DefaultAvatarUrl);
        }
    }
}

void FSafeOSSv1Caller::OnGetAvatarUrlComplete(
    bool bWasSuccessful,
    FString ResultUrl,
    TWeakPtr<IOnlineAvatar> AvatarInterface,
    TSharedRef<bool> EventFired,
    FOnGetAvatarUrlComplete OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (!*EventFired)
    {
        *EventFired = true;
        OnComplete.ExecuteIfBound(bWasSuccessful, ResultUrl);
    }
}

void FSafeOSSv1Caller::GetFriendsList(
    const TSharedRef<IOnlineFriends> &FriendsInterface,
    int32 LocalUserNum,
    const FString &ListName,
    TArray<TSharedRef<FOnlineFriend>> &OutFriends)
{
    if (!FriendsInterface->GetFriendsList(LocalUserNum, ListName, OutFriends))
    {
        OutFriends.Empty();
    }
}

void FSafeOSSv1Caller::GetRecentPlayers(
    const TSharedRef<IOnlineFriends> &FriendsInterface,
    const FUniqueNetId &UserId,
    const FString &Namespace,
    TArray<TSharedRef<FOnlineRecentPlayer>> &OutRecentPlayers)
{
    if (!FriendsInterface->GetRecentPlayers(UserId, Namespace, OutRecentPlayers))
    {
        OutRecentPlayers.Empty();
    }
}

void FSafeOSSv1Caller::GetBlockedPlayers(
    const TSharedRef<IOnlineFriends> &FriendsInterface,
    const FUniqueNetId &UserId,
    TArray<TSharedRef<FOnlineBlockedPlayer>> &OutBlockedPlayers)
{
    if (!FriendsInterface->GetBlockedPlayers(UserId, OutBlockedPlayers))
    {
        OutBlockedPlayers.Empty();
    }
}

void FSafeOSSv1Caller::ReadFriendsList(
    const TSharedRef<IOnlineFriends> &FriendsInterface,
    int32 LocalUserNum,
    const FString &ListName,
    const FOnReadFriendsListComplete &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    auto EventFired = MakeShared<bool>();
    if (!FriendsInterface->ReadFriendsList(
            LocalUserNum,
            ListName,
            FOnReadFriendsListComplete::CreateStatic(
                &FSafeOSSv1Caller::OnReadFriendsListComplete,
                TWeakPtr<IOnlineFriends>(FriendsInterface),
                LocalUserNum,
                ListName,
                EventFired,
                OnComplete)))
    {
        if (!*EventFired)
        {
            *EventFired = true;
            OnComplete.ExecuteIfBound(LocalUserNum, false, ListName, TEXT("Asynchronous call failed to start."));
        }
    }
}

void FSafeOSSv1Caller::OnReadFriendsListComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FString &ListName,
    const FString &ErrorStr,
    TWeakPtr<IOnlineFriends> FriendsInterface,
    int32 RequestedLocalUserNum,
    FString RequestedListName,
    TSharedRef<bool> EventFired,
    FOnReadFriendsListComplete OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    // @note: We do not compare ListName != RequestedListName because some OSSv1 implementations do not guarantee the
    // input value is passed through unmodified.
    if (LocalUserNum != RequestedLocalUserNum)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%d != %d): %s"),
            LocalUserNum,
            RequestedLocalUserNum,
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    if (!*EventFired)
    {
        *EventFired = true;
        OnComplete.ExecuteIfBound(LocalUserNum, bWasSuccessful, ListName, ErrorStr);
    }
}

void FSafeOSSv1Caller::QueryBlockedPlayers(
    const TSharedRef<IOnlineFriends> &FriendsInterface,
    const FUniqueNetId &LocalUserId,
    const FOnQueryBlockedPlayersCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = FriendsInterface->AddOnQueryBlockedPlayersCompleteDelegate_Handle(
        FOnQueryBlockedPlayersCompleteDelegate::CreateLambda(
            [&bDidExecuteCallback](const FUniqueNetId &, bool, const FString &) {
                bDidExecuteCallback = true;
            }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle = FriendsInterface->AddOnQueryBlockedPlayersCompleteDelegate_Handle(
        FOnQueryBlockedPlayersCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnQueryBlockedPlayersComplete,
            TWeakPtr<IOnlineFriends>(FriendsInterface),
            LocalUserId.ToString(),
            DelegateHandle,
            OnComplete));

    auto bStarted = FriendsInterface->QueryBlockedPlayers(LocalUserId);
    FriendsInterface->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            FriendsInterface->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(LocalUserId, false, TEXT("Asynchronous call failed to start."));
        }
    }
}

void FSafeOSSv1Caller::OnQueryBlockedPlayersComplete(
    const FUniqueNetId &UserId,
    bool bWasSuccessful,
    const FString &Error,
    TWeakPtr<IOnlineFriends> FriendsInterface,
    FString RequestedLocalUserIdString,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnQueryBlockedPlayersCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (UserId.ToString() != RequestedLocalUserIdString)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%s != %s): %s"),
            *UserId.ToString(),
            *RequestedLocalUserIdString,
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto FriendsInterfacePinned = FriendsInterface.Pin();
    if (FriendsInterfacePinned.IsValid())
    {
        FriendsInterfacePinned->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(UserId, bWasSuccessful, Error);
}

void FSafeOSSv1Caller::QueryRecentPlayers(
    const TSharedRef<IOnlineFriends> &FriendsInterface,
    const FUniqueNetId &LocalUserId,
    const FString &Namespace,
    const FOnQueryRecentPlayersCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = FriendsInterface->AddOnQueryRecentPlayersCompleteDelegate_Handle(
        FOnQueryRecentPlayersCompleteDelegate::CreateLambda(
            [&bDidExecuteCallback](const FUniqueNetId &, const FString &, bool, const FString &) {
                bDidExecuteCallback = true;
            }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle = FriendsInterface->AddOnQueryRecentPlayersCompleteDelegate_Handle(
        FOnQueryRecentPlayersCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnQueryRecentPlayersComplete,
            TWeakPtr<IOnlineFriends>(FriendsInterface),
            LocalUserId.ToString(),
            Namespace,
            DelegateHandle,
            OnComplete));

    auto bStarted = FriendsInterface->QueryRecentPlayers(LocalUserId, Namespace);
    FriendsInterface->ClearOnQueryRecentPlayersCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            FriendsInterface->ClearOnQueryRecentPlayersCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(LocalUserId, Namespace, false, TEXT("Asynchronous call failed to start."));
        }
    }
}

void FSafeOSSv1Caller::OnQueryRecentPlayersComplete(
    const FUniqueNetId &UserId,
    const FString &Namespace,
    bool bWasSuccessful,
    const FString &Error,
    TWeakPtr<IOnlineFriends> FriendsInterface,
    FString RequestedLocalUserIdString,
    FString RequestedNamespace,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnQueryRecentPlayersCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    // @note: We do not compare Namespace != RequestedNamespace as a precaution because we've observed some OSSv1
    // implementations not guaranteeing that ListName for ReadFriendsList is passed through unmodified.
    if (UserId.ToString() != RequestedLocalUserIdString)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%s != %s): %s"),
            *UserId.ToString(),
            *RequestedLocalUserIdString,
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto FriendsInterfacePinned = FriendsInterface.Pin();
    if (FriendsInterfacePinned.IsValid())
    {
        FriendsInterfacePinned->ClearOnQueryRecentPlayersCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(UserId, Namespace, bWasSuccessful, Error);
}

bool FSafeOSSv1Caller::ShowInviteUI(
    const TSharedRef<IOnlineExternalUI> &ExternalUIInterface,
    int32 LocalUserNum,
    const FName &SessionName)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    return ExternalUIInterface->ShowInviteUI(LocalUserNum, SessionName);
}

void FSafeOSSv1Caller::ShowLoginUI(
    const TSharedRef<IOnlineExternalUI> &ExternalUIInterface,
    int ControllerIndex,
    bool bShowOnlineOnly,
    bool bShowSkipButton,
    const FOnLoginUIClosedDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    auto EventFired = MakeShared<bool>();
    if (!ExternalUIInterface->ShowLoginUI(
            ControllerIndex,
            bShowOnlineOnly,
            bShowSkipButton,
            FOnLoginUIClosedDelegate::CreateStatic(
                &FSafeOSSv1Caller::OnLoginUIClosed,
                TWeakPtr<IOnlineExternalUI>(ExternalUIInterface),
                EventFired,
                OnComplete)))
    {
        if (!*EventFired)
        {
            *EventFired = true;
            OnComplete.ExecuteIfBound(
                nullptr,
                ControllerIndex,
                FOnlineError::CreateError(
                    TEXT("ShowLoginUI"),
                    EOnlineErrorResult::Unknown,
                    TEXT("not_signed_in"),
                    FText::FromString("User is not signed in, and could not show external UI sign-in prompt.")));
        }
    }
}

void FSafeOSSv1Caller::OnLoginUIClosed(
    TSharedPtr<const FUniqueNetId> UniqueId,
    int ControllerIndex,
    const FOnlineError &Error,
    TWeakPtr<IOnlineExternalUI> ExternalUIInterface,
    TSharedRef<bool> EventFired,
    FOnLoginUIClosedDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (!*EventFired)
    {
        *EventFired = true;
        OnComplete.ExecuteIfBound(UniqueId, ControllerIndex, Error);
    }
}

void FSafeOSSv1Caller::Login(
    const TSharedRef<IOnlineIdentity> &IdentityInterface,
    int LocalUserNum,
    const FOnlineAccountCredentials &Credentials,
    const FOnLoginCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
        LocalUserNum,
        FOnLoginCompleteDelegate::CreateLambda(
            [&bDidExecuteCallback](int32, bool, const FUniqueNetId &, const FString &) {
                bDidExecuteCallback = true;
            }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
        LocalUserNum,
        FOnLoginCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnLoginComplete,
            TWeakPtr<IOnlineIdentity>(IdentityInterface),
            LocalUserNum,
            DelegateHandle,
            OnComplete));

    auto bStarted = IdentityInterface->Login(LocalUserNum, Credentials);
    IdentityInterface->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            IdentityInterface->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, *DelegateHandle);
            OnComplete.ExecuteIfBound(
                LocalUserNum,
                false,
                *FUniqueNetIdString::EmptyId(),
                TEXT("Asynchronous call failed to start."));
        }
    }
}

void FSafeOSSv1Caller::OnLoginComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &UserId,
    const FString &Error,
    TWeakPtr<IOnlineIdentity> IdentityInterface,
    int32 RequestedLocalUserNum,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnLoginCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (LocalUserNum != RequestedLocalUserNum)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%d != %d): %s"),
            LocalUserNum,
            RequestedLocalUserNum,
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto IdentityInterfacePinned = IdentityInterface.Pin();
    if (IdentityInterfacePinned.IsValid())
    {
        IdentityInterfacePinned->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, *DelegateHandle);
    }
    OnComplete.ExecuteIfBound(LocalUserNum, bWasSuccessful, UserId, Error);
}

void FSafeOSSv1Caller::RegisterPlayers(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FName &SessionName,
    const TArray<FUniqueNetIdRef> &PlayerIds,
    const FOnRegisterPlayersCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle =
        SessionInterface->AddOnRegisterPlayersCompleteDelegate_Handle(FOnRegisterPlayersCompleteDelegate::CreateLambda(
            [&bDidExecuteCallback](FName, const TArray<FUniqueNetIdRef> &, bool) {
                bDidExecuteCallback = true;
            }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle =
        SessionInterface->AddOnRegisterPlayersCompleteDelegate_Handle(FOnRegisterPlayersCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnRegisterPlayersComplete,
            TWeakPtr<IOnlineSession>(SessionInterface),
            SessionName,
            DelegateHandle,
            OnComplete));

    auto bStarted = SessionInterface->RegisterPlayers(SessionName, PlayerIds, false);
    SessionInterface->ClearOnRegisterPlayersCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            SessionInterface->ClearOnRegisterPlayersCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(SessionName, PlayerIds, false);
        }
    }
}

TTask<bool> FSafeOSSv1Caller::RegisterPlayersAsync(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FName &SessionName,
    const TArray<FUniqueNetIdRef> &PlayerIds)
{
    auto DeferredTask = TTask<bool>::Deferred();

    FSafeOSSv1Caller::RegisterPlayers(
        SessionInterface,
        SessionName,
        PlayerIds,
        FOnRegisterPlayersCompleteDelegate::CreateLambda(
            [DeferredTask](FName, const TArray<FUniqueNetIdRef> &, bool bWasSuccessful) {
                DeferredTask.SetValue(bWasSuccessful);
            }));

    return DeferredTask;
}

void FSafeOSSv1Caller::OnRegisterPlayersComplete(
    FName SessionName,
    const TArray<FUniqueNetIdRef> &PlayerIds,
    bool bWasSuccessful,
    TWeakPtr<IOnlineSession> SessionInterface,
    FName RequestedSessionName,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnRegisterPlayersCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (SessionName != RequestedSessionName)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%s != %s): %s"),
            *SessionName.ToString(),
            *RequestedSessionName.ToString(),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto SessionInterfacePinned = SessionInterface.Pin();
    if (SessionInterfacePinned.IsValid())
    {
        SessionInterfacePinned->ClearOnRegisterPlayersCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(SessionName, PlayerIds, bWasSuccessful);
}

void FSafeOSSv1Caller::UnregisterPlayers(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FName &SessionName,
    const TArray<FUniqueNetIdRef> &PlayerIds,
    const FOnRegisterPlayersCompleteDelegate &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe call: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    bool bDidExecuteCallback = false;
    FDelegateHandle BadOSSWorkaroundHandle = SessionInterface->AddOnUnregisterPlayersCompleteDelegate_Handle(
        FOnUnregisterPlayersCompleteDelegate::CreateLambda(
            [&bDidExecuteCallback](FName, const TArray<FUniqueNetIdRef> &, bool) {
                bDidExecuteCallback = true;
            }));

    auto DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle = SessionInterface->AddOnUnregisterPlayersCompleteDelegate_Handle(
        FOnUnregisterPlayersCompleteDelegate::CreateStatic(
            &FSafeOSSv1Caller::OnUnregisterPlayersComplete,
            TWeakPtr<IOnlineSession>(SessionInterface),
            SessionName,
            DelegateHandle,
            OnComplete));

    auto bStarted = SessionInterface->UnregisterPlayers(SessionName, PlayerIds);
    SessionInterface->ClearOnUnregisterPlayersCompleteDelegate_Handle(BadOSSWorkaroundHandle);
    if (!bStarted)
    {
        if (!bDidExecuteCallback)
        {
            SessionInterface->ClearOnUnregisterPlayersCompleteDelegate_Handle(*DelegateHandle);
            OnComplete.ExecuteIfBound(SessionName, PlayerIds, false);
        }
    }
}

TTask<bool> FSafeOSSv1Caller::UnregisterPlayersAsync(
    const TSharedRef<IOnlineSession> &SessionInterface,
    const FName &SessionName,
    const TArray<FUniqueNetIdRef> &PlayerIds)
{
    auto DeferredTask = TTask<bool>::Deferred();

    FSafeOSSv1Caller::UnregisterPlayers(
        SessionInterface,
        SessionName,
        PlayerIds,
        FOnUnregisterPlayersCompleteDelegate::CreateLambda(
            [DeferredTask](FName, const TArray<FUniqueNetIdRef> &, bool bWasSuccessful) {
                DeferredTask.SetValue(bWasSuccessful);
            }));

    return DeferredTask;
}

void FSafeOSSv1Caller::OnUnregisterPlayersComplete(
    FName SessionName,
    const TArray<FUniqueNetIdRef> &PlayerIds,
    bool bWasSuccessful,
    TWeakPtr<IOnlineSession> SessionInterface,
    FName RequestedSessionName,
    TSharedRef<FDelegateHandle> DelegateHandle,
    FOnUnregisterPlayersCompleteDelegate OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationOSSv1, VeryVerbose, TEXT("Safe event: %s"), ANSI_TO_TCHAR(__FUNCTION__));

    if (SessionName != RequestedSessionName)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Safe event dropped (%s != %s): %s"),
            *SessionName.ToString(),
            *RequestedSessionName.ToString(),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    auto SessionInterfacePinned = SessionInterface.Pin();
    if (SessionInterfacePinned.IsValid())
    {
        SessionInterfacePinned->ClearOnUnregisterPlayersCompleteDelegate_Handle(*DelegateHandle);
    }
    OnComplete.ExecuteIfBound(SessionName, PlayerIds, bWasSuccessful);
}

};

REDPOINT_EOS_CODE_GUARD_END()