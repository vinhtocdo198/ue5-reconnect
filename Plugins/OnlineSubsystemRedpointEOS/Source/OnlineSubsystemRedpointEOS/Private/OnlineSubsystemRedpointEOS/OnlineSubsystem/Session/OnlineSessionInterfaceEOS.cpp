// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/OnlineSessionInterfaceEOS.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Legacy/SyntheticSessionManager.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/AttributeNames.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/OnlineSessionInfoEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/OnlineSessionSearchResultEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/UniqueNetIdEOSSession.h"
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#include "RedpointEOSAPI/SessionModification/AddAttribute.h"
#include "RedpointEOSAPI/SessionModification/RemoveAttribute.h"
#include "RedpointEOSAPI/SessionModification/SetBucketId.h"
#include "RedpointEOSAPI/SessionModification/SetHostAddress.h"
#include "RedpointEOSAPI/SessionModification/SetInvitesAllowed.h"
#include "RedpointEOSAPI/SessionModification/SetJoinInProgressAllowed.h"
#include "RedpointEOSAPI/SessionModification/SetMaxPlayers.h"
#include "RedpointEOSAPI/SessionModification/SetPermissionLevel.h"
#include "RedpointEOSAPI/Sessions/CreateSessionModification.h"
#include "RedpointEOSAPI/Sessions/DestroySession.h"
#include "RedpointEOSAPI/Sessions/UpdateSession.h"
#include "RedpointEOSAPI/Sessions/UpdateSessionModification.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSNetworking/NetworkingSystem.h"
#include "RedpointEOSOrchestrator/OrchestratorSystem.h"

#include "Engine/Engine.h"
#include "Misc/CommandLine.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSErrorConv.h"
#include "OnlineSubsystemUtils.h"
#include "SocketSubsystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow-uncaptured-local"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#endif

namespace REDPOINT_EOS_FILE_NS_ID(809401287, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session)
{
using namespace ::Redpoint::EOS::API::Sessions;
using namespace ::Redpoint::EOS::API::SessionModification;
using namespace ::Redpoint::EOS::API::LobbySessionsShared;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Error;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy;
#endif
using namespace ::Redpoint::EOS::Orchestrator;

FOnlineSessionInterfaceEOS::FOnlineSessionInterfaceEOS(
    const FPlatformHandle &InPlatformHandle,
    const FOnlineIdentityInterfaceEOSRef &InIdentity,
    const TSharedPtr<IOnlineFriends, ESPMode::ThreadSafe> &InFriends,
    const TSharedRef<IConfig> &InConfig)
    : PlatformHandle(InPlatformHandle)
    , Identity(InIdentity)
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    , Friends(InFriends)
#endif
    , Config(InConfig)
    , SessionLock()
    , Sessions()
    , Unregister_JoinSessionAccepted()
    , Unregister_SessionInviteAccepted()
    , Unregister_SessionInviteReceived()
{
}

void FOnlineSessionInterfaceEOS::Handle_SessionAddressChanged(
    const UE::Online::FAccountId &ListeningUserId,
    const TSharedPtr<const FInternetAddr> &InternetAddr,
    const TArray<TSharedRef<const FInternetAddr>> &DeveloperInternetAddrs)
{
    if (InternetAddr.IsValid())
    {
        auto TargetUserIdEOS = GetUniqueNetIdRef(ListeningUserId);

        FScopeLock ScopeLock(&this->SessionLock);
        for (int32 SearchIndex = 0; SearchIndex < this->Sessions.Num(); SearchIndex++)
        {
            auto OwnerUserId = this->Sessions[SearchIndex].OwningUserId;
            if (!OwnerUserId.IsValid() || !OwnerUserId->GetType().IsEqual(REDPOINT_EOS_SUBSYSTEM))
            {
                continue;
            }

            if (*StaticCastSharedPtr<const FUniqueNetIdEOS>(OwnerUserId) != *TargetUserIdEOS)
            {
                continue;
            }

            // We found a session that is impacted by the address change, perform an UpdateSession operation
            // to set it's HostAddress.

            EOS_EResult ModHandleResult;
            auto ModHandle = FUpdateSessionModification::Execute(
                this->PlatformHandle,
                FUpdateSessionModification::Options{.SessionName = this->Sessions[SearchIndex].SessionName.ToString()},
                ModHandleResult);
            if (ModHandleResult != EOS_EResult::EOS_Success)
            {
                UE_LOG(
                    LogRedpointEOS,
                    Error,
                    TEXT("Handle_SessionAddressChanged: Could not create session modification handle to update "
                         "address"));
                continue;
            }

            bool bIsPeerToPeerSession;
            EOS_EResult ApplyResult = this->ApplyConnectionSettingsToModificationHandle(
                this->Sessions[SearchIndex].SessionName,
                InternetAddr.ToSharedRef(),
                DeveloperInternetAddrs,
                ModHandle,
                this->Sessions[SearchIndex].SessionSettings,
                bIsPeerToPeerSession);
            if (ApplyResult != EOS_EResult::EOS_Success)
            {
                UE_LOG(
                    LogRedpointEOS,
                    Error,
                    TEXT("Handle_SessionAddressChanged: Could not apply settings to session handle (%s)"),
                    ANSI_TO_TCHAR(EOS_EResult_ToString(ApplyResult)));
                continue;
            }

            FUpdateSession::Execute(
                this->PlatformHandle,
                FUpdateSession::Options{.SessionModificationHandle = ModHandle},
                FUpdateSession::CompletionDelegate::CreateSPLambda(this, [](const FUpdateSession::Result &Info) {
                    if (Info.ResultCode != EOS_EResult::EOS_Success)
                    {
                        UE_LOG(
                            LogRedpointEOS,
                            Error,
                            TEXT("Handle_SessionAddressChanged: Failed to update host address, got error %s"),
                            ANSI_TO_TCHAR(EOS_EResult_ToString(Info.ResultCode)));
                        return;
                    }

                    UE_LOG(
                        LogRedpointEOS,
                        Verbose,
                        TEXT("Handle_SessionAddressChanged: Updated host address successfully"));
                }));
        }
    }
    else
    {
        auto TargetUserIdEOS = GetUniqueNetIdRef(ListeningUserId);

        FScopeLock ScopeLock(&this->SessionLock);
        for (int32 SearchIndex = 0; SearchIndex < this->Sessions.Num(); SearchIndex++)
        {
            auto OwnerUserId = this->Sessions[SearchIndex].OwningUserId;
            if (!OwnerUserId.IsValid() || !OwnerUserId->GetType().IsEqual(REDPOINT_EOS_SUBSYSTEM))
            {
                continue;
            }

            if (*StaticCastSharedPtr<const FUniqueNetIdEOS>(OwnerUserId) != *TargetUserIdEOS)
            {
                continue;
            }

            // We found a session that is impacted by the address change, perform an UpdateSession operation
            // to remove it's listening status.

            EOS_EResult ModHandleResult;
            auto ModHandle = FUpdateSessionModification::Execute(
                this->PlatformHandle,
                FUpdateSessionModification::Options{.SessionName = this->Sessions[SearchIndex].SessionName.ToString()},
                ModHandleResult);
            if (ModHandleResult != EOS_EResult::EOS_Success)
            {
                UE_LOG(
                    LogRedpointEOS,
                    Error,
                    TEXT("Handle_SessionAddressClosed: Could not create session modification handle to update "
                         "address"));
                continue;
            }

            // Mark the session as *not* listening so clients will no longer discover it in search results.
            {
                UE_LOG(
                    LogRedpointEOS,
                    Verbose,
                    TEXT("Session '%s' is being set as 'not listening' in response to the Handle_SessionAddressClosed "
                         "event."),
                    *this->Sessions[SearchIndex].SessionName.ToString());

                auto Result = FAddAttribute::Execute(
                    ModHandle,
                    FAddAttribute::Options{
                        .AttributeData = FAttributeData(EOS_WELL_KNOWN_ATTRIBUTE_BLISTENING, false),
                        .AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise});
                if (Result != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("Handle_SessionAddressClosed: Can not update session as no longer listening for "
                             "connections"));
                    return;
                }
            }

            FUpdateSession::Execute(
                this->PlatformHandle,
                FUpdateSession::Options{.SessionModificationHandle = ModHandle},
                FUpdateSession::CompletionDelegate::CreateSPLambda(this, [](const FUpdateSession::Result &Info) {
                    if (Info.ResultCode != EOS_EResult::EOS_Success)
                    {
                        UE_LOG(
                            LogRedpointEOS,
                            Error,
                            TEXT("Handle_SessionAddressClosed: Failed to update host address, got error %s"),
                            ANSI_TO_TCHAR(EOS_EResult_ToString(Info.ResultCode)));
                        return;
                    }

                    UE_LOG(
                        LogRedpointEOS,
                        Verbose,
                        TEXT("Handle_SessionAddressClosed: Updated host address successfully"));
                }));
        }
    }
}

void FOnlineSessionInterfaceEOS::RegisterEvents()
{
    auto NetworkingSystem = this->PlatformHandle->GetSystem<INetworkingSystem>();
    NetworkingSystem->OnListeningStateChanged().AddThreadSafeSP(
        this,
        &FOnlineSessionInterfaceEOS::Handle_SessionAddressChanged);

    EOS_Sessions_AddNotifyJoinSessionAcceptedOptions OptsJoinSessionAccepted = {};
    OptsJoinSessionAccepted.ApiVersion = EOS_SESSIONS_ADDNOTIFYJOINSESSIONACCEPTED_API_LATEST;
    EOS_Sessions_AddNotifySessionInviteAcceptedOptions OptsSessionInviteAccepted = {};
    OptsSessionInviteAccepted.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITEACCEPTED_API_LATEST;
    EOS_Sessions_AddNotifySessionInviteReceivedOptions OptsSessionInviteReceived = {};
    OptsSessionInviteReceived.ApiVersion = EOS_SESSIONS_ADDNOTIFYSESSIONINVITERECEIVED_API_LATEST;

    this->Unregister_JoinSessionAccepted = EOSRegisterEvent<
        EOS_HSessions,
        EOS_Sessions_AddNotifyJoinSessionAcceptedOptions,
        EOS_Sessions_JoinSessionAcceptedCallbackInfo>(
        this->PlatformHandle->Get<EOS_HSessions>(),
        &OptsJoinSessionAccepted,
        EOS_Sessions_AddNotifyJoinSessionAccepted,
        EOS_Sessions_RemoveNotifyJoinSessionAccepted,
        [WeakThis = GetWeakThis(this)](const EOS_Sessions_JoinSessionAcceptedCallbackInfo *Data) {
            if (auto This = PinWeakThis(WeakThis))
            {
                This->Handle_JoinSessionAccepted(Data);
            }
        });
    this->Unregister_SessionInviteAccepted = EOSRegisterEvent<
        EOS_HSessions,
        EOS_Sessions_AddNotifySessionInviteAcceptedOptions,
        EOS_Sessions_SessionInviteAcceptedCallbackInfo>(
        this->PlatformHandle->Get<EOS_HSessions>(),
        &OptsSessionInviteAccepted,
        EOS_Sessions_AddNotifySessionInviteAccepted,
        EOS_Sessions_RemoveNotifySessionInviteAccepted,
        [WeakThis = GetWeakThis(this)](const EOS_Sessions_SessionInviteAcceptedCallbackInfo *Data) {
            if (auto This = PinWeakThis(WeakThis))
            {
                This->Handle_SessionInviteAccepted(Data);
            }
        });
    this->Unregister_SessionInviteReceived = EOSRegisterEvent<
        EOS_HSessions,
        EOS_Sessions_AddNotifySessionInviteReceivedOptions,
        EOS_Sessions_SessionInviteReceivedCallbackInfo>(
        this->PlatformHandle->Get<EOS_HSessions>(),
        &OptsSessionInviteReceived,
        EOS_Sessions_AddNotifySessionInviteReceived,
        EOS_Sessions_RemoveNotifySessionInviteReceived,
        [WeakThis = GetWeakThis(this)](const EOS_Sessions_SessionInviteReceivedCallbackInfo *Data) {
            if (auto This = PinWeakThis(WeakThis))
            {
                This->Handle_SessionInviteReceived(Data);
            }
        });
}

void FOnlineSessionInterfaceEOS::Handle_JoinSessionAccepted(const EOS_Sessions_JoinSessionAcceptedCallbackInfo *Data)
{
    // We have joined a session via the overlay.

    UE_LOG(LogRedpointEOS, Verbose, TEXT("Received JoinSessionAccepted event from EOS Sessions system"));

    EOS_HSessionDetails Details = {};
    EOS_Sessions_CopySessionHandleByUiEventIdOptions CopyOpts = {};
    CopyOpts.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEBYUIEVENTID_API_LATEST;
    CopyOpts.UiEventId = Data->UiEventId;
    auto Result =
        EOS_Sessions_CopySessionHandleByUiEventId(this->PlatformHandle->Get<EOS_HSessions>(), &CopyOpts, &Details);
    if (Result != EOS_EResult::EOS_Success)
    {
        EOS_UI_AcknowledgeEventIdOptions AckOpts = {};
        AckOpts.ApiVersion = EOS_UI_ACKNOWLEDGECORRELATIONID_API_LATEST;
        AckOpts.Result = Result;
        AckOpts.UiEventId = Data->UiEventId;
        EOS_UI_AcknowledgeEventId(this->PlatformHandle->Get<EOS_HUI>(), &AckOpts);
        return;
    }

    FOnlineSessionSearchResultEOS SearchResult =
        FOnlineSessionSearchResultEOS::CreateFromDetails(this->PlatformHandle, Details);

    TSharedPtr<FUniqueNetIdEOS> LocalUserEOS = MakeShared<FUniqueNetIdEOS>(Data->LocalUserId);

    int32 LocalUserNum = 0;
    if (!this->Identity->GetLocalUserNum(*LocalUserEOS, LocalUserNum))
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("Join session notification in JoinSessionAccepted was not for a locally logged in user"));
        EOS_UI_AcknowledgeEventIdOptions AckOpts = {};
        AckOpts.ApiVersion = EOS_UI_ACKNOWLEDGECORRELATIONID_API_LATEST;
        AckOpts.Result = EOS_EResult::EOS_UnexpectedError;
        AckOpts.UiEventId = Data->UiEventId;
        EOS_UI_AcknowledgeEventId(this->PlatformHandle->Get<EOS_HUI>(), &AckOpts);
        return;
    }

    this->TriggerOnSessionUserInviteAcceptedDelegates(true, LocalUserNum, LocalUserEOS, SearchResult);

    UE_LOG(
        LogRedpointEOS,
        Verbose,
        TEXT("Acknowledging join session accepted event after firing OnSessionUserInviteAccepted"));
    EOS_UI_AcknowledgeEventIdOptions AckOpts = {};
    AckOpts.ApiVersion = EOS_UI_ACKNOWLEDGECORRELATIONID_API_LATEST;
    AckOpts.Result = EOS_EResult::EOS_Success;
    AckOpts.UiEventId = Data->UiEventId;
    EOS_UI_AcknowledgeEventId(this->PlatformHandle->Get<EOS_HUI>(), &AckOpts);
}

void FOnlineSessionInterfaceEOS::Handle_SessionInviteAccepted(
    const EOS_Sessions_SessionInviteAcceptedCallbackInfo *Data)
{
    // We have accepted an invite from another user via the overlay.

    UE_LOG(
        LogRedpointEOS,
        Verbose,
        TEXT("Received SessionInviteAccepted event from EOS Sessions system: %s"),
        ANSI_TO_TCHAR(Data->InviteId));

    EOS_HSessionDetails Details = {};
    EOS_Sessions_CopySessionHandleByInviteIdOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST;
    Opts.InviteId = Data->InviteId;
    if (EOS_Sessions_CopySessionHandleByInviteId(this->PlatformHandle->Get<EOS_HSessions>(), &Opts, &Details) !=
        EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("Failed to retrieve session details from invite ID during SessionInviteAccepted"));
        return;
    }

    FOnlineSessionSearchResultEOS SearchResult =
        FOnlineSessionSearchResultEOS::CreateFromDetails(this->PlatformHandle, Details);

    TSharedPtr<FUniqueNetIdEOS> TargetUserEOS = MakeShared<FUniqueNetIdEOS>(Data->TargetUserId);
    TSharedPtr<FUniqueNetIdEOS> LocalUserEOS = MakeShared<FUniqueNetIdEOS>(Data->LocalUserId);

    int32 LocalUserNum = 0;
    if (!this->Identity->GetLocalUserNum(*LocalUserEOS, LocalUserNum))
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("Accepted session invite in SessionInviteAccepted was not for a locally logged in user"));
        return;
    }

    this->TriggerOnSessionUserInviteAcceptedDelegates(true, LocalUserNum, LocalUserEOS, SearchResult);
}

void FOnlineSessionInterfaceEOS::Handle_SessionInviteReceived(
    const EOS_Sessions_SessionInviteReceivedCallbackInfo *Data)
{
    // We have received an invite from another user.

    UE_LOG(
        LogRedpointEOS,
        Verbose,
        TEXT("Received SessionInviteReceived event from EOS Sessions system: %s"),
        ANSI_TO_TCHAR(Data->InviteId));

    EOS_HSessionDetails Details = {};
    EOS_Sessions_CopySessionHandleByInviteIdOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST;
    Opts.InviteId = Data->InviteId;
    if (EOS_Sessions_CopySessionHandleByInviteId(this->PlatformHandle->Get<EOS_HSessions>(), &Opts, &Details) !=
        EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("Failed to retrieve session details from invite ID during SessionInviteReceived"));
        return;
    }

    FOnlineSessionSearchResultEOS SearchResult =
        FOnlineSessionSearchResultEOS::CreateFromDetails(this->PlatformHandle, Details);

    TSharedPtr<FUniqueNetIdEOS> SentUserEOS = MakeShared<FUniqueNetIdEOS>(Data->TargetUserId);
    TSharedPtr<FUniqueNetIdEOS> ReceivedUserEOS = MakeShared<FUniqueNetIdEOS>(Data->LocalUserId);

    int32 LocalUserNum = 0;
    if (!this->Identity->GetLocalUserNum(*ReceivedUserEOS, LocalUserNum))
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("Accepted session invite in SessionInviteReceived was not for a locally logged in user"));
        return;
    }

    this->TriggerOnSessionInviteReceivedDelegates(*ReceivedUserEOS, *SentUserEOS, TEXT(""), SearchResult);
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FOnlineSessionInterfaceEOS::ShouldHaveSyntheticSession(const FOnlineSessionSettings &SessionSettings) const
{
    if (this->Config->GetPresenceAdvertisementType() != EPresenceAdvertisementType::Session)
    {
        // Game does not advertise sessions via presence.
        return false;
    }

    return SessionSettings.bUsesPresence;
}

#endif

FNamedOnlineSession *FOnlineSessionInterfaceEOS::AddNamedSession(
    FName SessionName,
    const FOnlineSessionSettings &SessionSettings)
{
    FScopeLock ScopeLock(&this->SessionLock);
    return new (this->Sessions) FNamedOnlineSession(SessionName, SessionSettings);
}

FNamedOnlineSession *FOnlineSessionInterfaceEOS::AddNamedSession(FName SessionName, const FOnlineSession &Session)
{
    FScopeLock ScopeLock(&this->SessionLock);
    return new (this->Sessions) FNamedOnlineSession(SessionName, Session);
}

TSharedPtr<const FUniqueNetId> FOnlineSessionInterfaceEOS::CreateSessionIdFromString(const FString &SessionIdStr)
{
    return MakeShared<FUniqueNetIdEOSSession>(SessionIdStr);
}

FNamedOnlineSession *FOnlineSessionInterfaceEOS::GetNamedSession(FName SessionName)
{
    FScopeLock ScopeLock(&this->SessionLock);
    for (int32 SearchIndex = 0; SearchIndex < this->Sessions.Num(); SearchIndex++)
    {
        if (this->Sessions[SearchIndex].SessionName == SessionName)
        {
            return &this->Sessions[SearchIndex];
        }
    }

    return nullptr;
}

void FOnlineSessionInterfaceEOS::RemoveNamedSession(FName SessionName)
{
    FScopeLock ScopeLock(&this->SessionLock);
    for (int32 SearchIndex = 0; SearchIndex < this->Sessions.Num(); SearchIndex++)
    {
        if (this->Sessions[SearchIndex].SessionName == SessionName)
        {
            this->Sessions.RemoveAtSwap(SearchIndex);
            return;
        }
    }
}

bool FOnlineSessionInterfaceEOS::HasPresenceSession()
{
    FScopeLock ScopeLock(&this->SessionLock);
    for (int32 SearchIndex = 0; SearchIndex < this->Sessions.Num(); SearchIndex++)
    {
        if (this->Sessions[SearchIndex].SessionSettings.bUsesPresence)
        {
            return true;
        }
    }

    return false;
}

EOnlineSessionState::Type FOnlineSessionInterfaceEOS::GetSessionState(FName SessionName) const
{
    FScopeLock ScopeLock(&this->SessionLock);
    for (int32 SearchIndex = 0; SearchIndex < this->Sessions.Num(); SearchIndex++)
    {
        if (this->Sessions[SearchIndex].SessionName == SessionName)
        {
            return this->Sessions[SearchIndex].SessionState;
        }
    }

    return EOnlineSessionState::NoSession;
}

bool FOnlineSessionInterfaceEOS::CreateSession(
    int32 HostingPlayerNum,
    FName SessionName,
    const FOnlineSessionSettings &NewSessionSettings)
{
    auto Id = this->Identity->GetUniquePlayerId(HostingPlayerNum);
    if (!Id.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("HostingPlayerNum provided to CreateSession does not have online identity."));
        return false;
    }

    return this->CreateSession(Id.ToSharedRef().Get(), SessionName, NewSessionSettings);
}

FString FOnlineSessionInterfaceEOS::GetBucketId(const FOnlineSessionSettings &SessionSettings)
{
    auto GameMode = SessionSettings.Settings.Contains(SETTING_GAMEMODE)
                        ? SessionSettings.Settings[SETTING_GAMEMODE].Data.ToString()
                        : TEXT("<None>");
    auto Region = SessionSettings.Settings.Contains(SETTING_REGION)
                      ? SessionSettings.Settings[SETTING_REGION].Data.ToString()
                      : TEXT("<None>");
    auto MapName = SessionSettings.Settings.Contains(SETTING_MAPNAME)
                       ? SessionSettings.Settings[SETTING_MAPNAME].Data.ToString()
                       : TEXT("<None>");

    FString BucketId = FString::Printf(TEXT("%s:%s:%s"), *GameMode, *Region, *MapName);
    if (BucketId.Len() > 60)
    {
        // The EOS backend limits bucket IDs to 60 characters.
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("The bucket ID '%s' is too long for the EOS backend, which limits bucket IDs to a maximum of 60 "
                 "characters. The bucket ID '%s' will be used instead."),
            *BucketId,
            *BucketId.Mid(0, 60));
        BucketId = BucketId.Mid(0, 60);
    }
    return BucketId;
}

EOS_EResult FOnlineSessionInterfaceEOS::ApplyConnectionSettingsToModificationHandle(
    const FName &SessionName,
    const TSharedRef<const FInternetAddr> &InternetAddr,
    const TArray<TSharedRef<const FInternetAddr>> &DeveloperInternetAddrs,
    TSharedRef<TReleasable<EOS_HSessionModification>> Handle,
    const FOnlineSessionSettings &SessionSettings,
    bool &bIsPeerToPeerAddress)
{
    FString HostAddressStr = InternetAddr->ToString(true);
    TArray<FString> DeveloperAddressesArray;
    for (const auto &DevAddr : DeveloperInternetAddrs)
    {
        DeveloperAddressesArray.Add(DevAddr->ToString(false));
    }
    FString DeveloperAddressesStr = FString::Join(DeveloperAddressesArray, TEXT(","));
    if (InternetAddr->GetProtocolType().IsEqual(REDPOINT_EOS_SUBSYSTEM))
    {
        // If this is a P2P connection, we just override the host address entirely.
        auto Result = FSetHostAddress::Execute(
            Handle,
            FSetHostAddress::Options{
                .HostAddress = HostAddressStr,
            });
        if (Result != EOS_EResult::EOS_Success)
        {
            return Result;
        }
        bIsPeerToPeerAddress = true;
    }
    else
    {
        // Check to see if the user has overridden the port that we should expose
        // in the AddressBound attribute. We only use this attribute if the EOS_IGNORE_ORCHESTRATOR_PORT
        // environment variable is not enabled (which can be used to prevent server orchestrators overriding
        // the main game port for in-cluster testing).
        FString OverrideAddress = TEXT("");
        bool bUseOverrideAddress = false;
        FString IgnoreOverridePort = FPlatformMisc::GetEnvironmentVariable(TEXT("EOS_IGNORE_ORCHESTRATOR_PORT"));
        if (IgnoreOverridePort != TEXT("true"))
        {
            if (SessionSettings.Settings.Contains(FName(EOS_WELL_KNOWN_ATTRIBUTE_OVERRIDEADDRESSBOUND)))
            {
                SessionSettings.Settings.Find(FName(EOS_WELL_KNOWN_ATTRIBUTE_OVERRIDEADDRESSBOUND))
                    ->Data.GetValue(OverrideAddress);
                bUseOverrideAddress = true;
            }
        }

        // If this is an IP-based connection, we explicitly *don't* set the HostAddress, because
        // we want EOS to populate that field with the true public IP address. But we still need
        // to include own address information so we can compute the port to connect to (which EOS
        // does not know about).
        auto Result = FAddAttribute::Execute(
            Handle,
            FAddAttribute::Options{
                .AttributeData = FAttributeData(
                    EOS_WELL_KNOWN_ATTRIBUTE_ADDRESS_BOUND,
                    bUseOverrideAddress ? OverrideAddress : HostAddressStr),
                .AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise,
            });
        if (Result != EOS_EResult::EOS_Success)
        {
            return Result;
        }

        // If the EOS_OVERRIDE_HOST_IP environment variable is set, this forcibly overrides any IP
        // detection that the EOS backend will do for IP-based connections. The value in this environment
        // variable will be stored in the Address attribute. The port detection provided by server
        // orchestrators will still apply.
        FString OverrideHostIP = FPlatformMisc::GetEnvironmentVariable(TEXT("EOS_OVERRIDE_HOST_IP"));
        if (!OverrideHostIP.IsEmpty())
        {
            UE_LOG(LogRedpointEOS, Verbose, TEXT("Using host IP from EOS_OVERRIDE_HOST_IP: %s"), *OverrideHostIP);
            auto OverrideResult = FSetHostAddress::Execute(
                Handle,
                FSetHostAddress::Options{
                    .HostAddress = OverrideHostIP,
                });
            if (OverrideResult != EOS_EResult::EOS_Success)
            {
                UE_LOG(
                    LogRedpointEOS,
                    Error,
                    TEXT("Failed to set host address based on EOS_OVERRIDE_HOST_IP environment variable: %s"),
                    *::ConvertError(OverrideResult).ToLogString());
                return OverrideResult;
            }
        }
        // This is an "else if", since we don't want to do any development IP resolution if the
        // host IP is being provided by an environment variable.
#if !UE_BUILD_SHIPPING
        else if (
            this->PlatformHandle->GetEnvironment()->Config->GetUseFallbackAddressesInDevelopment() &&
            !FParse::Param(FCommandLine::Get(), TEXT("emulateeosshipping")))
        {
            // If we have at least one developer address, set the "AddressDev" attribute as well. This
            // field is used so that play-in-editor and other machines on the same local area network
            // can connect to a dedicated server during development.
            if (DeveloperInternetAddrs.Num() > 0)
            {
                auto DevResult = FAddAttribute::Execute(
                    Handle,
                    FAddAttribute::Options{
                        .AttributeData = FAttributeData(EOS_WELL_KNOWN_ATTRIBUTE_ADDRESS_DEV, DeveloperAddressesStr),
                        .AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise,
                    });
                if (DevResult != EOS_EResult::EOS_Success)
                {
                    return DevResult;
                }
            }
        }
#endif
    }

    // Mark the session as listening so we can discover it in search results.
    {
        UE_LOG(
            LogRedpointEOSSessionListening,
            Verbose,
            TEXT("Session '%s' is being set as 'listening' in the ApplyConnectionSettingsToModificationHandle function "
                 "as listening addresses are available."),
            *SessionName.ToString());

        auto Result = FAddAttribute::Execute(
            Handle,
            FAddAttribute::Options{
                .AttributeData = FAttributeData(EOS_WELL_KNOWN_ATTRIBUTE_BLISTENING, true),
                .AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise});
        if (Result != EOS_EResult::EOS_Success)
        {
            return Result;
        }
    }

    return EOS_EResult::EOS_Success;
}

EOS_EResult FOnlineSessionInterfaceEOS::ApplySettingsToModificationHandle(
    const FName &SessionName,
    const TSharedPtr<const FUniqueNetIdEOS> &HostingUserId,
    const FOnlineSessionSettings &SessionSettings,
    TSharedRef<TReleasable<EOS_HSessionModification>> Handle,
    const FOnlineSessionSettings *ExistingSessionSettings)
{
    // If the hosting user ID hasn't been set, assume a dedicated server. This makes "CreateSession" have the correct
    // networking behaviour when HostingUserId hasn't been passed in on a dedicated server.
    auto HostingAccountId =
        HostingUserId.IsValid() ? GetAccountId(HostingUserId.ToSharedRef()) : GetDedicatedServerAccountId();

    // Try to update the host address with information passed in through the net driver.
    bool bIsPeerToPeerAddress = false;
    TSharedPtr<const FInternetAddr> HostAddress;
    TArray<TSharedRef<const FInternetAddr>> DeveloperAddresses;
    this->PlatformHandle->GetSystem<INetworkingSystem>()->GetListeningState(
        HostingAccountId,
        HostAddress,
        DeveloperAddresses);
    if (HostAddress.IsValid())
    {
        EOS_EResult ConnectionSettingsResult = this->ApplyConnectionSettingsToModificationHandle(
            SessionName,
            HostAddress.ToSharedRef(),
            DeveloperAddresses,
            Handle,
            SessionSettings,
            bIsPeerToPeerAddress);
        if (ConnectionSettingsResult != EOS_EResult::EOS_Success)
        {
            return ConnectionSettingsResult;
        }
    }
    else
    {
#if !REDPOINT_EOS_UE_5_5_OR_LATER
        // Make it clearer for developers when this situation is caused by not using the
        // EOS net driver (due to misconfiguration). Detect if the EOS net driver has been
        // set as the networking driver properly.
        if (GEngine != nullptr)
        {
            bool bDidFindEOSNetDriver = false;
            for (auto Definition : GEngine->NetDriverDefinitions)
            {
                if (Definition.DefName.IsEqual(FName(TEXT("GameNetDriver"))) &&
                    Definition.DriverClassName.IsEqual(
                        FName(TEXT("/Script/RedpointEOSNetworking.RedpointEOSNetDriver"))))
                {
                    bDidFindEOSNetDriver = true;
                    break;
                }
            }
            if (!bDidFindEOSNetDriver)
            {
                // Do not emit this error during automated testing, because it will cause session tests to
                // fail unnecessarily.
                if (!this->Config->IsAutomatedTesting())
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT(
                            "You are not using the EOS networking driver, so the host address can not be set correctly "
                            "during the CreateSession or UpdateSession call. In addition, players will not be able "
                            "connect over P2P. Fix your configuration to correctly set up networking: "
                            "https://docs.redpoint.games/eos-online-subsystem/docs/"
                            "core_configuration#enabling-epic-online-services"));
                }
            }
        }
#endif

        // NOTE: We no longer emit a warning now, since creating a session before starting the server is now
        // a valid operation to do.

        // Mark the session as *not* listening so clients won't discover it in search results.
        {
            UE_LOG(
                LogRedpointEOSSessionListening,
                Verbose,
                TEXT("Session '%s' is being set as 'not listening' in the ApplySettingsToModificationHandle "
                     "function as there is no listening address available for this session."),
                *SessionName.ToString());

            auto Result = FAddAttribute::Execute(
                Handle,
                FAddAttribute::Options{
                    .AttributeData = FAttributeData(EOS_WELL_KNOWN_ATTRIBUTE_BLISTENING, false),
                    .AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise});
            if (Result != EOS_EResult::EOS_Success)
            {
                UE_LOG(
                    LogRedpointEOS,
                    Error,
                    TEXT("EOS_SessionModification_AddAttribute operation failed when setting '%s' attribute to %s "
                         "(result code: %s)"),
                    ANSI_TO_TCHAR(EOS_WELL_KNOWN_ATTRIBUTE_BLISTENING),
                    TEXT("false"),
                    ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
                return Result;
            }
        }
    }

    {
        auto Result =
            FSetBucketId::Execute(Handle, FSetBucketId::Options{.BucketId = this->GetBucketId(SessionSettings)});
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT(
                    "EOS_SessionModification_SetBucketId operation failed when setting to bucket ID '%s' (result code: "
                    "%s)"),
                *this->GetBucketId(SessionSettings),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return Result;
        }
    }

    {
        auto Result = FSetMaxPlayers::Execute(
            Handle,
            FSetMaxPlayers::Options{.MaxPlayers = static_cast<uint32>(SessionSettings.NumPublicConnections)});
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("EOS_SessionModification_SetMaxPlayers operation failed when setting number of players to %d "
                     "(result code: "
                     "%s)"),
                SessionSettings.NumPublicConnections,
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return Result;
        }
    }

    {
        auto Result = FSetJoinInProgressAllowed::Execute(
            Handle,
            FSetJoinInProgressAllowed::Options{.bAllowJoinInProgress = SessionSettings.bAllowJoinInProgress});
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("EOS_SessionModification_SetJoinInProgressAllowed operation failed when join in progress to %s "
                     "(result code: "
                     "%s)"),
                SessionSettings.bAllowJoinInProgress ? TEXT("true") : TEXT("false"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return Result;
        }
    }

    {
        EOS_EOnlineSessionPermissionLevel PermissionLevel;
        if (SessionSettings.bShouldAdvertise)
        {
            PermissionLevel = EOS_EOnlineSessionPermissionLevel::EOS_OSPF_PublicAdvertised;
        }
        else if (SessionSettings.bAllowJoinViaPresence)
        {
            PermissionLevel = EOS_EOnlineSessionPermissionLevel::EOS_OSPF_JoinViaPresence;
        }
        else if (SessionSettings.bAllowInvites)
        {
            PermissionLevel = EOS_EOnlineSessionPermissionLevel::EOS_OSPF_InviteOnly;
        }
        else
        {
            // Default to invite only. If we don't do this, it would default to PublicAdvertised which is definitely not
            // what you want if bShouldAdvertise is false.
            PermissionLevel = EOS_EOnlineSessionPermissionLevel::EOS_OSPF_InviteOnly;
        }

        auto Result =
            FSetPermissionLevel::Execute(Handle, FSetPermissionLevel::Options{.PermissionLevel = PermissionLevel});
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("EOS_SessionModification_SetPermissionLevel operation failed when setting permission level to %d "
                     "(result code: %s)"),
                PermissionLevel,
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return Result;
        }
    }

    {
        auto Result = FSetInvitesAllowed::Execute(
            Handle,
            FSetInvitesAllowed::Options{.bInvitesAllowed = SessionSettings.bAllowInvites});
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("EOS_SessionModification_SetInvitesAllowed operation failed when setting invites allowed to %s "
                     "(result code: %s)"),
                SessionSettings.bAllowInvites ? TEXT("true") : TEXT("false"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return Result;
        }
    }

    if (ExistingSessionSettings == nullptr)
    {
        // We only set __EOS_bUsesPresence on create, because EOS does not actually support
        // changing this setting at the API level later.

        auto Result = FAddAttribute::Execute(
            Handle,
            FAddAttribute::Options{
                .AttributeData = FAttributeData(EOS_WELL_KNOWN_ATTRIBUTE_BUSESPRESENCE, SessionSettings.bUsesPresence),
                .AdvertisementType = EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise,
            });
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("EOS_SessionModification_AddAttribute operation failed when setting '%s' attribute to %s (result "
                     "code: %s)"),
                ANSI_TO_TCHAR(EOS_WELL_KNOWN_ATTRIBUTE_BUSESPRESENCE),
                SessionSettings.bUsesPresence ? TEXT("true") : TEXT("false"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return Result;
        }
    }

    for (const auto &Setting : SessionSettings.Settings)
    {
        FAttributeData Attribute;

        auto KeyStr = EOSString_SessionModification_AttributeKey::ToAnsiString(Setting.Key.ToString());
        auto ValueStr = EOSString_SessionModification_AttributeStringValue::ToUtf8String(Setting.Value.Data.ToString());

        // Set attribute value.
        switch (Setting.Value.Data.GetType())
        {
        case EOnlineKeyValuePairDataType::Bool: {
            bool BoolVal;
            Setting.Value.Data.GetValue(BoolVal);
            Attribute = FAttributeData(Setting.Key.ToString(), BoolVal);
            break;
        }
        case EOnlineKeyValuePairDataType::Int64: {
            int64 Int64Val;
            Setting.Value.Data.GetValue(Int64Val);
            Attribute = FAttributeData(Setting.Key.ToString(), Int64Val);
            break;
        }
        case EOnlineKeyValuePairDataType::Double: {
            double DoubleVal;
            Setting.Value.Data.GetValue(DoubleVal);
            Attribute = FAttributeData(Setting.Key.ToString(), DoubleVal);
            break;
        }
        case EOnlineKeyValuePairDataType::Float: {
            float FloatVal;
            Setting.Value.Data.GetValue(FloatVal);
            Attribute = FAttributeData(Setting.Key.ToString(), static_cast<double>(FloatVal));
            break;
        }
        case EOnlineKeyValuePairDataType::String: {
            FString StringVal = Setting.Value.Data.ToString();
            if (StringVal.Len() == 0)
            {
                UE_LOG(
                    LogRedpointEOS,
                    Warning,
                    TEXT("EOS_SessionModification_AddAttribute called for string attribute '%s', but the string value "
                         "has a length of 0 - this will probably fail!"),
                    *Attribute.Key());
            }
            Attribute = FAttributeData(Setting.Key.ToString(), StringVal);
            break;
        }
        default:
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("ApplySettingsToModificationHandle: Unsupported data type %s for search parameter %s"),
                Setting.Value.Data.GetTypeString(),
                *Setting.Key.ToString());
            return EOS_EResult::EOS_InvalidParameters;
        }

        auto Result = FAddAttribute::Execute(
            Handle,
            FAddAttribute::Options{
                .AttributeData = Attribute,
                .AdvertisementType =
                    Setting.Value.AdvertisementType >= EOnlineDataAdvertisementType::Type::ViaOnlineService
                        ? EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise
                        : EOS_ESessionAttributeAdvertisementType::EOS_SAAT_DontAdvertise,
            });
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("EOS_SessionModification_AddAttribute operation failed when setting '%s' attribute (result "
                     "code: %s)"),
                *Attribute.Key(),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return Result;
        }
    }

    if (ExistingSessionSettings != nullptr)
    {
        for (const auto &OldSetting : ExistingSessionSettings->Settings)
        {
            if (!SessionSettings.Settings.Contains(OldSetting.Key))
            {
                auto Result = FRemoveAttribute::Execute(
                    Handle,
                    FRemoveAttribute::Options{.AttributeKey = OldSetting.Key.ToString()});

                if (Result != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("EOS_SessionModification_RemoveAttribute operation failed when setting '%s' attribute "
                             "(result code: %s)"),
                        *OldSetting.Key.ToString(),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
                    return Result;
                }
            }
        }
    }

    return EOS_EResult::EOS_Success;
}

void FOnlineSessionInterfaceEOS::MetricsSend_BeginPlayerSession(
    const FUniqueNetIdEOS &UserId,
    const FString &SessionId,
    const FString &GameServerAddress)
{
    auto ProductUserIdAnsi = EOSString_ProductUserId::ToAnsiString(UserId.GetProductUserId());
    auto UserAccount = this->Identity->GetUserAccount(UserId);
    FString DisplayName = TEXT("");
    if (UserAccount.IsValid())
    {
        DisplayName = UserAccount->GetDisplayName();
    }
    auto DisplayNameUtf8 = EOSString_UserInfo_DisplayName::ToUtf8String(DisplayName);
    auto SessionIdAnsi = StringCast<ANSICHAR>(*SessionId);
    auto GameServerAddressAnsi = StringCast<ANSICHAR>(*GameServerAddress);

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (!UserId.IsDedicatedServer())
    {
        EOS_Metrics_BeginPlayerSessionOptions MetricsOpts = {};
        MetricsOpts.ApiVersion = EOS_METRICS_BEGINPLAYERSESSION_API_LATEST;
        TSharedPtr<const Redpoint::EOS::Auth::CrossPlatform::FCrossPlatformAccountId> CrossPlatformAccountId =
            this->Identity->GetCrossPlatformAccountId(UserId);
        if (CrossPlatformAccountId.IsValid() && CrossPlatformAccountId->GetType() == EPIC_GAMES_ACCOUNT_ID)
        {
            EOS_EpicAccountId EpicAccountId =
                StaticCastSharedPtr<const Redpoint::EOS::Auth::CrossPlatform::FEpicGamesCrossPlatformAccountId>(
                    CrossPlatformAccountId)
                    ->GetEpicAccountId();
            if (EOSString_EpicAccountId::IsValid(EpicAccountId))
            {
                MetricsOpts.AccountIdType = EOS_EMetricsAccountIdType::EOS_MAIT_Epic;
                MetricsOpts.AccountId.Epic = EpicAccountId;
            }
            else
            {
                MetricsOpts.AccountIdType = EOS_EMetricsAccountIdType::EOS_MAIT_External;
                MetricsOpts.AccountId.External = ProductUserIdAnsi.Ptr.Get();
            }
        }
        else
        {
            MetricsOpts.AccountIdType = EOS_EMetricsAccountIdType::EOS_MAIT_External;
            MetricsOpts.AccountId.External = ProductUserIdAnsi.Ptr.Get();
        }
        MetricsOpts.ControllerType = EOS_EUserControllerType::EOS_UCT_Unknown;
        MetricsOpts.DisplayName = DisplayNameUtf8.GetAsChar();
        MetricsOpts.GameSessionId = SessionIdAnsi.Get();
        MetricsOpts.ServerIp = GameServerAddressAnsi.Get();
        auto MetricsResult = EOS_Metrics_BeginPlayerSession(this->PlatformHandle->Get<EOS_HMetrics>(), &MetricsOpts);
        if (MetricsResult != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Warning,
                TEXT("EOS_Metrics_BeginPlayerSession failed: %s"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(MetricsResult)));
        }
    }
#endif
}

void FOnlineSessionInterfaceEOS::MetricsSend_EndPlayerSession(const FUniqueNetIdEOS &UserId)
{
    auto ProductUserIdAnsi = EOSString_ProductUserId::ToAnsiString(UserId.GetProductUserId());

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (!UserId.IsDedicatedServer())
    {
        EOS_Metrics_EndPlayerSessionOptions MetricsOpts = {};
        MetricsOpts.ApiVersion = EOS_METRICS_ENDPLAYERSESSION_API_LATEST;
        TSharedPtr<const Redpoint::EOS::Auth::CrossPlatform::FCrossPlatformAccountId> CrossPlatformAccountId =
            this->Identity->GetCrossPlatformAccountId(UserId);
        if (CrossPlatformAccountId.IsValid() && CrossPlatformAccountId->GetType() == EPIC_GAMES_ACCOUNT_ID)
        {
            EOS_EpicAccountId EpicAccountId =
                StaticCastSharedPtr<const Redpoint::EOS::Auth::CrossPlatform::FEpicGamesCrossPlatformAccountId>(
                    CrossPlatformAccountId)
                    ->GetEpicAccountId();
            if (EOSString_EpicAccountId::IsValid(EpicAccountId))
            {
                MetricsOpts.AccountIdType = EOS_EMetricsAccountIdType::EOS_MAIT_Epic;
                MetricsOpts.AccountId.Epic = EpicAccountId;
            }
            else
            {
                MetricsOpts.AccountIdType = EOS_EMetricsAccountIdType::EOS_MAIT_External;
                MetricsOpts.AccountId.External = ProductUserIdAnsi.Ptr.Get();
            }
        }
        else
        {
            MetricsOpts.AccountIdType = EOS_EMetricsAccountIdType::EOS_MAIT_External;
            MetricsOpts.AccountId.External = ProductUserIdAnsi.Ptr.Get();
        }
        auto MetricsResult = EOS_Metrics_EndPlayerSession(this->PlatformHandle->Get<EOS_HMetrics>(), &MetricsOpts);
        if (MetricsResult != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Warning,
                TEXT("EOS_Metrics_EndPlayerSession failed: %s"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(MetricsResult)));
        }
    }
#endif
}

static TTask<void>::With<ETaskBinding::Static> UpdateSessionSettingsWithOrchestrator(
    FPlatformHandle PlatformHandle,
    FName SessionName,
    TSharedRef<FOnlineSessionSettings> NewSessionSettings)
{
    auto PortMappings = co_await PlatformHandle->GetSystem<IOrchestratorSystem>()->GetPortMappings();

    for (const auto &PortMapping : PortMappings)
    {
        if (PortMapping.GetPortName().IsEqual(NAME_GamePort) || PortMapping.GetPortName().IsEqual(NAME_None) ||
            PortMapping.GetPortName().IsEqual(NAME_Default) || PortMapping.GetPortName().IsEqual(FName("game")) ||
            PortMapping.GetPortName().IsEqual(FName("default")))
        {
            NewSessionSettings->Settings.Add(
                EOS_WELL_KNOWN_ATTRIBUTE_OVERRIDEADDRESSBOUND,
                FOnlineSessionSetting(
                    FString::Printf(TEXT("0.0.0.0:%d"), PortMapping.GetPortValue()),
                    EOnlineDataAdvertisementType::ViaOnlineService));
        }
        else
        {
            NewSessionSettings->Settings.Add(
                FName(*FString::Printf(TEXT("Port_%s"), *PortMapping.GetPortName().ToString())),
                FOnlineSessionSetting(
                    FString::Printf(TEXT("%d"), PortMapping.GetPortValue()),
                    EOnlineDataAdvertisementType::ViaOnlineService));
        }
    }

    co_return;
}

bool FOnlineSessionInterfaceEOS::CreateSession(
    const FUniqueNetId &HostingPlayerId,
    FName SessionName,
    const FOnlineSessionSettings &NewSessionSettings)
{
    if (HostingPlayerId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("CreateSession: Hosting user ID was invalid"));
        return false;
    }

    auto HostingPlayerIdEOS = StaticCastSharedRef<const FUniqueNetIdEOS>(HostingPlayerId.AsShared());
    auto NewSessionSettingsRef = MakeShared<FOnlineSessionSettings>(NewSessionSettings);

    AsCallback(
        UpdateSessionSettingsWithOrchestrator(this->PlatformHandle, SessionName, NewSessionSettingsRef),
        [WeakThis = GetWeakThis(this), HostingPlayerIdEOS, SessionName, NewSessionSettingsRef]() {
            if (auto This = PinWeakThis(WeakThis))
            {
                This->CreateSessionInternal(*HostingPlayerIdEOS, SessionName, *NewSessionSettingsRef);
            }
        });
    return true;
}

TTask<void> FOnlineSessionInterfaceEOS::FinalizeForCreateSessionInternal(
    FError Error,
    TSharedRef<FUniqueNetIdEOSSession> SessionId,
    FName SessionName,
    TSharedRef<const FUniqueNetIdEOS> EOSHostingUser)
{
    co_await this->PlatformHandle->GetSystem<IOrchestratorSystem>()->SessionCreated(SessionName, SessionId->ToString());

    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        UE_LOG(LogRedpointEOS, Warning, TEXT("CreateSession: Session no longer exists when Finalize was running."));
        this->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
        co_return;
    }

    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("CreateSession: One or more optional operations failed: %s"),
            *Error.ToLogString());
    }

    UE_LOG(
        LogRedpointEOS,
        Verbose,
        TEXT("CreateSession: Successfully created session '%s' with ID '%s'"),
        *SessionName.ToString(),
        *SessionId->ToString());
    Session->SessionState = EOnlineSessionState::Pending;
    Session->SessionInfo = MakeShared<FOnlineSessionInfoEOS>(SessionId);
    this->TriggerOnCreateSessionCompleteDelegates(SessionName, true);
    this->MetricsSend_BeginPlayerSession(*EOSHostingUser, Session->GetSessionIdStr(), TEXT(""));
    co_return;
}

void FOnlineSessionInterfaceEOS::OnUpdateSessionCompleteForCreateSessionInternal(
    const FUpdateSession::Result &Result,
    FName SessionName,
    TSharedRef<const FUniqueNetIdEOS> EOSHostingUser,
    int32 SyntheticMaxPlayers)
{
    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("CreateSession: Session no longer exists when UpdateSession was running."));
        this->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
        return;
    }

    if (Result.ResultCode != EOS_EResult::EOS_Success)
    {
        this->RemoveNamedSession(SessionName);
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("CreateSession: Failed with error %s"),
            ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));
        this->TriggerOnCreateSessionCompleteDelegates(SessionName, false);
        return;
    }

    UE_LOG(
        LogRedpointEOS,
        Verbose,
        TEXT("CreateSession: Created session with name '%s' and ID '%s' on backend, now finalising "
             "session state in client..."),
        *SessionName.ToString(),
        *Result.SessionId);

    auto SessionId = MakeShared<FUniqueNetIdEOSSession>(Result.SessionId);

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->ShouldHaveSyntheticSession(Session->SessionSettings))
    {
        UE_LOG(
            LogRedpointEOS,
            Verbose,
            TEXT("CreateSession: Upserting synthetic session for newly created session..."));

        auto SPM = this->PlatformHandle->GetSystem<FSyntheticSessionManager>();
        SPM->UpsertSyntheticSession(
            SessionId,
            SyntheticMaxPlayers,
            FSyntheticSessionManager::FOnComplete::CreateSPLambda(
                this,
                [this, SessionId, SessionName, EOSHostingUser](const FError &Error) {
                    this->FinalizeForCreateSessionInternal(Error, SessionId, SessionName, EOSHostingUser);
                }));
    }
    else
    {
        this->FinalizeForCreateSessionInternal(Errors::Success(), SessionId, SessionName, EOSHostingUser);
    }
#else
    this->FinalizeForCreateSessionInternal(Errors::Success(), SessionId, SessionName, EOSHostingUser);
#endif
}

bool FOnlineSessionInterfaceEOS::CreateSessionInternal(
    const FUniqueNetId &HostingPlayerId,
    FName SessionName,
    const FOnlineSessionSettings &NewSessionSettings)
{
    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        Session = AddNamedSession(SessionName, NewSessionSettings);
        Session->SessionState = EOnlineSessionState::Creating;
        Session->OwningUserId = HostingPlayerId.AsShared();
        Session->LocalOwnerId = HostingPlayerId.AsShared();
        Session->bHosting = true;

        if (HostingPlayerId.GetType() != REDPOINT_EOS_SUBSYSTEM)
        {
            RemoveNamedSession(SessionName);
            UE_LOG(LogRedpointEOS, Error, TEXT("CreateSession: Hosting user ID was invalid"));
            return false;
        }

        auto EOSHostingUser = StaticCastSharedRef<const FUniqueNetIdEOS>(HostingPlayerId.AsShared());
        if (!EOSHostingUser->HasValidProductUserId())
        {
            RemoveNamedSession(SessionName);
            UE_LOG(LogRedpointEOS, Error, TEXT("CreateSession: Hosting user ID was invalid"));
            return false;
        }

        EOS_EResult ModResult;
        TSharedRef<TReleasable<EOS_HSessionModification>> ModHandle = FCreateSessionModification::Execute(
            this->PlatformHandle,
            FCreateSessionModification::Options{
                .SessionName = SessionName.ToString(),
                .BucketId = GetBucketId(NewSessionSettings),
                .MaxPlayers = static_cast<uint32>(NewSessionSettings.NumPublicConnections),
                .LocalUserId = EOSHostingUser->GetProductUserId(),
                .bPresenceEnabled =
                    this->Config->GetPresenceAdvertisementType() == EPresenceAdvertisementType::Session &&
                    NewSessionSettings.bUsesPresence,
                .SessionId = TEXT(""),
                .bSanctionsEnabled = false,
                .AllowedPlatformIds = TArray<Common::EOnlinePlatformType>(),
            },
            ModResult);
        if (ModResult != EOS_EResult::EOS_Success)
        {
            RemoveNamedSession(SessionName);
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("CreateSession: Could not create session modification handle: %s"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(ModResult)));
            return false;
        }

        EOS_EResult ApplyResult = this->ApplySettingsToModificationHandle(
            SessionName,
            EOSHostingUser,
            NewSessionSettings,
            ModHandle,
            nullptr);
        if (ApplyResult != EOS_EResult::EOS_Success)
        {
            RemoveNamedSession(SessionName);
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("CreateSession: Could not apply settings to session handle (%s)"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(ApplyResult)));
            return false;
        }

        UE_LOG(
            LogRedpointEOS,
            Verbose,
            TEXT("CreateSession: Creating session with name '%s'..."),
            *SessionName.ToString());
        FUpdateSession::Execute(
            this->PlatformHandle,
            FUpdateSession::Options{
                .SessionModificationHandle = ModHandle,
            },
            FUpdateSession::CompletionDelegate::CreateSP(
                this,
                &FOnlineSessionInterfaceEOS::OnUpdateSessionCompleteForCreateSessionInternal,
                SessionName,
                EOSHostingUser,
                NewSessionSettings.NumPublicConnections));
        return true;
    }
    else
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("CreateSession: Failed because a session with the name %s already exists."),
            *SessionName.ToString());
        return false;
    }
}

bool FOnlineSessionInterfaceEOS::StartSession(FName SessionName)
{
    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("StartSession: Called with non-existant session."));
        return false;
    }

    auto SessionNameAnsi = EOSString_SessionModification_SessionName::ToAnsiString(SessionName.ToString());

    EOS_Sessions_StartSessionOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONS_STARTSESSION_API_LATEST;
    Opts.SessionName = SessionNameAnsi.Ptr.Get();

    EOSRunOperation<EOS_HSessions, EOS_Sessions_StartSessionOptions, EOS_Sessions_StartSessionCallbackInfo>(
        this->PlatformHandle->Get<EOS_HSessions>(),
        &Opts,
        EOS_Sessions_StartSession,
        [WeakThis = GetWeakThis(this), SessionName](const EOS_Sessions_StartSessionCallbackInfo *Info) {
            if (auto This = PinWeakThis(WeakThis))
            {
                if (Info->ResultCode != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("StartSession: Failed with error %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Info->ResultCode)));
                }

                AsCallback(
                    This->PlatformHandle->GetSystem<IOrchestratorSystem>()->SessionStarted(),
                    [WeakThis = GetWeakThis(This), SessionName, ResultCode = Info->ResultCode]() {
                        if (auto This = PinWeakThis(WeakThis))
                        {
                            This->TriggerOnStartSessionCompleteDelegates(
                                SessionName,
                                ResultCode == EOS_EResult::EOS_Success);
                        }
                    });
            }
        });
    return true;
}

bool FOnlineSessionInterfaceEOS::UpdateSession(
    FName SessionName,
    FOnlineSessionSettings &UpdatedSessionSettings,
    bool bShouldRefreshOnlineData)
{
    auto UpdatedSessionSettingsRef = MakeShared<FOnlineSessionSettings>(UpdatedSessionSettings);

    AsCallback(
        UpdateSessionSettingsWithOrchestrator(this->PlatformHandle, SessionName, UpdatedSessionSettingsRef),
        [WeakThis = GetWeakThis(this), SessionName, UpdatedSessionSettingsRef, bShouldRefreshOnlineData]() {
            if (auto This = PinWeakThis(WeakThis))
            {
                This->UpdateSessionInternal(SessionName, *UpdatedSessionSettingsRef, bShouldRefreshOnlineData);
            }
        });
    return true;
}

void FOnlineSessionInterfaceEOS::OnUpdateSessionCompleteForUpdateSessionInternal(
    const FUpdateSession::Result &Result,
    FName SessionName)
{
    FNamedOnlineSession *SessionPtr = this->GetNamedSession(SessionName);
    if (SessionPtr == nullptr)
    {
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("UpdateSession: Ignoring result of UpdateSession operation because the session was "
                 "destroyed at the same time."));
        return;
    }

    if (Result.ResultCode != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("UpdateSession: Failed with error %s"),
            ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));
        this->TriggerOnUpdateSessionCompleteDelegates(SessionName, false);
        return;
    }

    UE_LOG(
        LogRedpointEOS,
        Verbose,
        TEXT("UpdateSession: Successfully updated session '%s' with ID '%s'"),
        *SessionName.ToString(),
        *Result.SessionId);
    SessionPtr->SessionState = EOnlineSessionState::Pending;
    SessionPtr->SessionInfo = MakeShared<FOnlineSessionInfoEOS>(MakeShared<FUniqueNetIdEOSSession>(Result.SessionId));
    this->TriggerOnUpdateSessionCompleteDelegates(SessionName, true);
    return;
}

bool FOnlineSessionInterfaceEOS::UpdateSessionInternal(
    FName SessionName,
    FOnlineSessionSettings &UpdatedSessionSettings,
    bool bShouldRefreshOnlineData)
{
    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("UpdateSession: Called with non-existant session."));
        return false;
    }

    if (!Session->OwningUserId.IsValid() || Session->OwningUserId->GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("UpdateSession: Hosting user ID was invalid"));
        return false;
    }

    auto EOSHostingUser = StaticCastSharedRef<const FUniqueNetIdEOS>(Session->OwningUserId.ToSharedRef());
    if (!EOSHostingUser->HasValidProductUserId())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("UpdateSession: Hosting user ID was invalid"));
        return false;
    }

    EOS_EResult ModHandleResult;
    auto ModHandle = FUpdateSessionModification::Execute(
        this->PlatformHandle,
        FUpdateSessionModification::Options{.SessionName = SessionName.ToString()},
        ModHandleResult);
    if (ModHandleResult != EOS_EResult::EOS_Success)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("UpdateSession: Could not create session modification handle"));
        return false;
    }

    EOS_EResult ApplyResult = this->ApplySettingsToModificationHandle(
        SessionName,
        EOSHostingUser,
        UpdatedSessionSettings,
        ModHandle,
        &Session->SessionSettings);
    if (ApplyResult != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("UpdateSession: Could not apply settings to session handle (%s)"),
            ANSI_TO_TCHAR(EOS_EResult_ToString(ApplyResult)));
        return false;
    }

    FUpdateSession::Execute(
        this->PlatformHandle,
        FUpdateSession::Options{.SessionModificationHandle = ModHandle},
        FUpdateSession::CompletionDelegate::CreateSP(
            this,
            &FOnlineSessionInterfaceEOS::OnUpdateSessionCompleteForUpdateSessionInternal,
            SessionName));
    return true;
}

bool FOnlineSessionInterfaceEOS::EndSession(FName SessionName)
{
    auto OrchestratorSystem = this->PlatformHandle->GetSystem<IOrchestratorSystem>();

    if (OrchestratorSystem->ShouldDestroySessionOnEndSession())
    {
        UE_LOG(
            LogRedpointEOS,
            Verbose,
            TEXT("Forwarding EndSession call to DestroySession, as this is required by the current server "
                 "orchestrator."));
        return this->DestroySession(
            SessionName,
            FOnDestroySessionCompleteDelegate::CreateLambda(
                [WeakThis = GetWeakThis(this), SessionName](FName DestroyedSessionName, bool bWasSuccessful) {
                    if (auto This = PinWeakThis(WeakThis))
                    {
                        if (SessionName == DestroyedSessionName)
                        {
                            // Forward EndSession event so that OSB nodes work correctly.
                            This->TriggerOnEndSessionCompleteDelegates(DestroyedSessionName, bWasSuccessful);
                        }
                    }
                }));
    }

    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("EndSession: Called with non-existant session."));
        return false;
    }

    auto SessionNameAnsi = EOSString_SessionModification_SessionName::ToAnsiString(SessionName.ToString());

    EOS_Sessions_EndSessionOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONS_ENDSESSION_API_LATEST;
    Opts.SessionName = SessionNameAnsi.Ptr.Get();

    EOSRunOperation<EOS_HSessions, EOS_Sessions_EndSessionOptions, EOS_Sessions_EndSessionCallbackInfo>(
        this->PlatformHandle->Get<EOS_HSessions>(),
        &Opts,
        EOS_Sessions_EndSession,
        [WeakThis = GetWeakThis(this), SessionName](const EOS_Sessions_EndSessionCallbackInfo *Info) {
            if (auto This = PinWeakThis(WeakThis))
            {
                if (Info->ResultCode != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("EndSession: Failed with error %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Info->ResultCode)));
                }

                AsCallback(
                    This->PlatformHandle->GetSystem<IOrchestratorSystem>()->SessionEnded(),
                    [WeakThis = GetWeakThis(This), SessionName, ResultCode = Info->ResultCode]() {
                        if (auto This = PinWeakThis(WeakThis))
                        {
                            This->TriggerOnEndSessionCompleteDelegates(
                                SessionName,
                                ResultCode == EOS_EResult::EOS_Success);
                        }
                    });
            }
        });
    return true;
}

bool FOnlineSessionInterfaceEOS::DestroySession(
    FName SessionName,
    const FOnDestroySessionCompleteDelegate &CompletionDelegate)
{
    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("DestroySession: Called with non-existant session."));
        return false;
    }
    if (Session->SessionInfo == nullptr)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("DestroySession: This session has already been destroyed."));
        return false;
    }

    auto UserIdEOS = StaticCastSharedPtr<const FUniqueNetIdEOS>(Session->LocalOwnerId);
    auto SessionId = StaticCastSharedRef<const FUniqueNetIdEOSSession>(Session->SessionInfo->GetSessionId().AsShared());

    auto Executor = MakeShared<FAsyncTaskGraph>();

    // Destroy the session.
    auto DestroyResult = MakeShared<FDestroySession::Result>();
    auto PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, SessionName, DestroyResult](const FAsyncTaskGraph::FOnComplete &OnComplete) {
                FDestroySession::Execute(
                    this->PlatformHandle,
                    FDestroySession::Options{SessionName.ToString()},
                    FDestroySession::CompletionDelegate::CreateSPLambda(
                        this,
                        [OnComplete, DestroyResult](const FDestroySession::Result &Result) {
                            if (Result.ResultCode != EOS_EResult::EOS_Success)
                            {
                                UE_LOG(
                                    LogRedpointEOS,
                                    Error,
                                    TEXT("%s"),
                                    *::Redpoint::EOS::API::ConvertError(
                                         ANSI_TO_TCHAR(__FUNCTION__),
                                         TEXT("Failed to destroy session."),
                                         Result.ResultCode)
                                         .ToLogString());
                            }

                            // Store the result so we can use the success code.
                            *DestroyResult = Result;

                            // We only log if the operation fails; we still proceed with the rest of the unwinding.
                            OnComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        FAsyncTaskGraph::ENoDependencies::StartImmediately);

    // End the player session.
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, UserIdEOS](const FAsyncTaskGraph::FOnComplete &OnComplete) {
                this->MetricsSend_EndPlayerSession(*UserIdEOS);
                OnComplete.ExecuteIfBound(Errors::Success());
            }),
        PreviousTask);

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    // Stop advertising the session through the synthetic session manager.
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, SessionId](const FAsyncTaskGraph::FOnComplete &OnComplete) {
                auto SPM = this->PlatformHandle->GetSystem<FSyntheticSessionManager>();
                SPM->DeleteSyntheticSession(
                    SessionId,
                    FSyntheticSessionManager::FOnComplete::CreateLambda([OnComplete](const FError &ErrorCode) {
                        if (!ErrorCode.WasSuccessful())
                        {
                            UE_LOG(LogRedpointEOS, Warning, TEXT("%s"), *ErrorCode.ToLogString());
                        }
                        OnComplete.ExecuteIfBound(Errors::Success());
                    }));
            }),
        PreviousTask);
#endif

    // Notify the server orchestrator. If we don't have one, it'll be the null server orchestrator implementation.
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, SessionId](const FAsyncTaskGraph::FOnComplete &OnComplete) {
                AsCallback(this->PlatformHandle->GetSystem<IOrchestratorSystem>()->SessionDestroyed(), [OnComplete]() {
                    OnComplete.ExecuteIfBound(Errors::Success());
                });
            }),
        PreviousTask);

    // Fire the events.
    PreviousTask = Executor->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, SessionName, DestroyResult, CompletionDelegate](const FAsyncTaskGraph::FOnComplete &OnComplete) {
                this->RemoveNamedSession(SessionName);
                this->TriggerOnDestroySessionCompleteDelegates(
                    SessionName,
                    DestroyResult->ResultCode == EOS_EResult::EOS_Success);
                CompletionDelegate.ExecuteIfBound(SessionName, DestroyResult->ResultCode == EOS_EResult::EOS_Success);
            }),
        PreviousTask);

    // Start the executor.
    Executor->Start();
    return true;
}

bool FOnlineSessionInterfaceEOS::IsPlayerInSession(FName SessionName, const FUniqueNetId &UniqueId)
{
    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("IsPlayerInSession: Called with non-existant session."));
        return false;
    }

    if (UniqueId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("IsPlayerInSession: Target user ID was invalid"));
        return false;
    }

    auto EOSTargetUser = StaticCastSharedRef<const FUniqueNetIdEOS>(UniqueId.AsShared());
    if (!EOSTargetUser->HasValidProductUserId())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("IsPlayerInSession: Target user ID was invalid"));
        return false;
    }

    auto SessionNameAnsi = EOSString_SessionModification_SessionName::ToAnsiString(SessionName.ToString());

    EOS_Sessions_IsUserInSessionOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONS_ISUSERINSESSION_API_LATEST;
    Opts.SessionName = SessionNameAnsi.Ptr.Get();
    Opts.TargetUserId = EOSTargetUser->GetProductUserId();

    EOS_EResult Result = EOS_Sessions_IsUserInSession(this->PlatformHandle->Get<EOS_HSessions>(), &Opts);
    switch (Result)
    {
    case EOS_EResult::EOS_Success:
        return true;
    case EOS_EResult::EOS_NotFound:
        return false;
    default:
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("IsPlayerInSession: Unexpected error %s"),
            ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
        return false;
    }
}

bool FOnlineSessionInterfaceEOS::StartMatchmaking(
    const TArray<TSharedRef<const FUniqueNetId>> &LocalPlayers,
    FName SessionName,
    const FOnlineSessionSettings &NewSessionSettings,
    TSharedRef<FOnlineSessionSearch> &SearchSettings)
{
    UE_LOG(
        LogRedpointEOS,
        Warning,
        TEXT("StartMatchmaking is not supported on this platform. Use FindSessions or FindSessionById."));
    this->TriggerOnMatchmakingCompleteDelegates(SessionName, false);
    return false;
}

bool FOnlineSessionInterfaceEOS::CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName)
{
    auto Id = this->Identity->GetUniquePlayerId(SearchingPlayerNum);
    if (!Id.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("SearchingPlayerNum provided to CancelMatchmaking does not have online identity."));
        return false;
    }

    return this->CancelMatchmaking(Id.ToSharedRef().Get(), SessionName);
}

bool FOnlineSessionInterfaceEOS::CancelMatchmaking(const FUniqueNetId &SearchingPlayerId, FName SessionName)
{
    UE_LOG(
        LogRedpointEOS,
        Warning,
        TEXT("CancelMatchmaking is not supported on this platform. Use CancelFindSessions."));
    this->TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
    return false;
}

bool FOnlineSessionInterfaceEOS::FindSessions(
    int32 SearchingPlayerNum,
    const TSharedRef<FOnlineSessionSearch> &SearchSettings)
{
    auto Id = this->Identity->GetUniquePlayerId(SearchingPlayerNum);
    if (!Id.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("SearchingPlayerNum provided to FindSessions does not have online identity."));
        return false;
    }

    return this->FindSessions(Id.ToSharedRef().Get(), SearchSettings);
}

bool FOnlineSessionInterfaceEOS::FindSessions(
    const FUniqueNetId &SearchingPlayerId,
    const TSharedRef<FOnlineSessionSearch> &SearchSettings)
{
    SearchSettings->SearchState = EOnlineAsyncTaskState::NotStarted;

    if (SearchingPlayerId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessions: Searching user ID was invalid"));
        return false;
    }

    auto EOSSearchingUser = StaticCastSharedRef<const FUniqueNetIdEOS>(SearchingPlayerId.AsShared());
    if (!EOSSearchingUser->HasValidProductUserId())
    {
        SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessions: Searching user ID was invalid"));
        return false;
    }

    EOS_Sessions_CreateSessionSearchOptions SearchOpts = {};
    SearchOpts.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
    SearchOpts.MaxSearchResults = static_cast<uint32_t>(SearchSettings->MaxSearchResults);

    EOS_HSessionSearch SearchHandle = {};
    if (EOS_Sessions_CreateSessionSearch(this->PlatformHandle->Get<EOS_HSessions>(), &SearchOpts, &SearchHandle) !=
        EOS_EResult::EOS_Success)
    {
        SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessions: Failed to create search handle"));
        return false;
    }

    bool bDidFilterListening = false;
    bool bDidFilterMinSlotsAvailable = false;

    for (const auto &Filter : SearchSettings->QuerySettings.SearchParams)
    {
        if (
#if !REDPOINT_EOS_UE_5_5_OR_LATER
            Filter.Key.IsEqual(SEARCH_PRESENCE) ||
#endif
            Filter.Key.IsEqual(SEARCH_LOBBIES))
        {
            // These are magic search keys that are only used by the Steam subsystem and
            // the Epic implementation of EOS. Ignore them in our plugin so that example
            // code written by Epic (Lyra) is able to search sessions correctly.
            continue;
        }

        if (Filter.Key.ToString() == EOS_WELL_KNOWN_ATTRIBUTE_BLISTENING)
        {
            bDidFilterListening = true;
        }

        if (Filter.Key.ToString() == EOS_SESSIONS_SEARCH_MINSLOTSAVAILABLE)
        {
            bDidFilterMinSlotsAvailable = true;
        }

        EOS_SessionSearch_SetParameterOptions ParamOpts = {};
        ParamOpts.ApiVersion = EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST;

        // Set comparison operator.
        switch (Filter.Value.ComparisonOp)
        {
        case EOnlineComparisonOp::Equals:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_EQUAL;
            break;
        case EOnlineComparisonOp::NotEquals:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_NOTEQUAL;
            break;
        case EOnlineComparisonOp::GreaterThan:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_GREATERTHAN;
            break;
        case EOnlineComparisonOp::GreaterThanEquals:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_GREATERTHANOREQUAL;
            break;
        case EOnlineComparisonOp::LessThan:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_LESSTHAN;
            break;
        case EOnlineComparisonOp::LessThanEquals:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_LESSTHANOREQUAL;
            break;
        case EOnlineComparisonOp::Near:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_DISTANCE;
            break;
        case EOnlineComparisonOp::In:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_ANYOF;
            break;
        case EOnlineComparisonOp::NotIn:
            ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_NOTANYOF;
            break;
        default:
            SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("FindSessions: Invalid comparison operation for search parameter %s"),
                *Filter.Key.ToString());
            return false;
        }

        auto KeyStr = EOSString_SessionModification_AttributeKey::ToAnsiString(Filter.Key.ToString());
        auto ValueStr = EOSString_SessionModification_AttributeStringValue::ToUtf8String(Filter.Value.Data.ToString());

        // Set attribute value.
        EOS_Sessions_AttributeData Attribute = {};
        Attribute.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
        Attribute.Key = KeyStr.Ptr.Get();
        switch (Filter.Value.Data.GetType())
        {
        case EOnlineKeyValuePairDataType::Bool:
            Attribute.ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
            bool BoolVal;
            Filter.Value.Data.GetValue(BoolVal);
            Attribute.Value.AsBool = BoolVal ? EOS_TRUE : EOS_FALSE;
            break;
        case EOnlineKeyValuePairDataType::Int64:
            Attribute.ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
            int64 Int64Val;
            Filter.Value.Data.GetValue(Int64Val);
            Attribute.Value.AsInt64 = Int64Val;
            break;
        case EOnlineKeyValuePairDataType::Double:
            Attribute.ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            Filter.Value.Data.GetValue(Attribute.Value.AsDouble);
            break;
        case EOnlineKeyValuePairDataType::Float:
            Attribute.ValueType = EOS_ESessionAttributeType::EOS_AT_DOUBLE;
            {
                float FloatTmp;
                Filter.Value.Data.GetValue(FloatTmp);
                Attribute.Value.AsDouble = static_cast<double>(FloatTmp);
            }
            break;
        case EOnlineKeyValuePairDataType::String:
            Attribute.ValueType = EOS_ESessionAttributeType::EOS_AT_STRING;
            Attribute.Value.AsUtf8 = ValueStr.GetAsChar();
            break;
        case EOnlineKeyValuePairDataType::Empty:
            if (Filter.Key.ToString() == EOS_WELL_KNOWN_ATTRIBUTE_BLISTENING)
            {
                // Developer wants to include both listening and non-listening sessions.
                break;
            }
            else
            {
                // Fallthrough as this is unhandled for anything other than EOS_WELL_KNOWN_ATTRIBUTE_BLISTENING.
            }
        default:
            SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("FindSessions: Unsupported data type %s for search parameter %s"),
                Filter.Value.Data.GetTypeString(),
                *Filter.Key.ToString());
            return false;
        }

        ParamOpts.Parameter = &Attribute;

        // Set search parameter.
        EOS_EResult SetParamResult = EOS_SessionSearch_SetParameter(SearchHandle, &ParamOpts);
        if (SetParamResult != EOS_EResult::EOS_Success)
        {
            SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("FindSessions: Failed to set search parameter %s due to error %s"),
                *Filter.Key.ToString(),
                ANSI_TO_TCHAR(EOS_EResult_ToString(SetParamResult)));
            return false;
        }
    }

    if (!bDidFilterListening)
    {
        // Exclude non-listening sessions from search results.
        EOS_Sessions_AttributeData Attribute = {};
        Attribute.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
        Attribute.Key = EOS_WELL_KNOWN_ATTRIBUTE_BLISTENING;
        Attribute.ValueType = EOS_ESessionAttributeType::EOS_AT_BOOLEAN;
        Attribute.Value.AsBool = EOS_TRUE;

        EOS_SessionSearch_SetParameterOptions ParamOpts = {};
        ParamOpts.ApiVersion = EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST;
        ParamOpts.Parameter = &Attribute;
        ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_EQUAL;

        EOS_EResult SetParamResult = EOS_SessionSearch_SetParameter(SearchHandle, &ParamOpts);
        if (SetParamResult != EOS_EResult::EOS_Success)
        {
            SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("FindSessions: Unable to set listening filter (got result %s)"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(SetParamResult)));
            return false;
        }
    }

    if (!bDidFilterMinSlotsAvailable &&
        Redpoint::EOS::Config::ApiVersionIsAtLeast(*this->Config.Get(), EEOSApiVersion::v2022_02_11))
    {
        // Require at least one slot by default. This has long been assumed to be the behaviour, based on
        // the comments in the EOS SDK. But in actuality, if you don't specify "minslotsavailable" as a
        // filter, the SDK will return you sessions that are full.
        //
        // Make our behaviour match the expectation of users.
        EOS_Sessions_AttributeData Attribute = {};
        Attribute.ApiVersion = EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST;
        Attribute.Key = EOS_SESSIONS_SEARCH_MINSLOTSAVAILABLE;
        Attribute.ValueType = EOS_ESessionAttributeType::EOS_AT_INT64;
        Attribute.Value.AsInt64 = 1;

        EOS_SessionSearch_SetParameterOptions ParamOpts = {};
        ParamOpts.ApiVersion = EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST;
        ParamOpts.Parameter = &Attribute;
        ParamOpts.ComparisonOp = EOS_EOnlineComparisonOp::EOS_CO_GREATERTHANOREQUAL;

        EOS_EResult SetParamResult = EOS_SessionSearch_SetParameter(SearchHandle, &ParamOpts);
        if (SetParamResult != EOS_EResult::EOS_Success)
        {
            SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("FindSessions: Unable to set minslotsavailable filter (got result %s)"),
                ANSI_TO_TCHAR(EOS_EResult_ToString(SetParamResult)));
            return false;
        }
    }

    SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;

    EOS_SessionSearch_FindOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
    Opts.LocalUserId = EOSSearchingUser->GetProductUserId();

    EOSRunOperation<EOS_HSessionSearch, EOS_SessionSearch_FindOptions, EOS_SessionSearch_FindCallbackInfo>(
        SearchHandle,
        &Opts,
        EOS_SessionSearch_Find,
        [WeakThis = GetWeakThis(this), SearchHandle, SearchSettings](const EOS_SessionSearch_FindCallbackInfo *Info) {
            if (auto This = PinWeakThis(WeakThis))
            {
                if (Info->ResultCode != EOS_EResult::EOS_Success)
                {
                    EOS_SessionSearch_Release(SearchHandle);
                    SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("FindSessions: Search failed with result code %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Info->ResultCode)));
                    This->TriggerOnFindSessionsCompleteDelegates(false);
                    return;
                }

                // Get the number of results.
                EOS_SessionSearch_GetSearchResultCountOptions CountOpts = {};
                CountOpts.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
                uint32_t ResultCount = EOS_SessionSearch_GetSearchResultCount(SearchHandle, &CountOpts);

                // Iterate through the search results and copy them all into the search settings.
                for (uint32_t i = 0; i < ResultCount; i++)
                {
                    EOS_SessionSearch_CopySearchResultByIndexOptions CopyOpts = {};
                    CopyOpts.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
                    CopyOpts.SessionIndex = i;

                    EOS_HSessionDetails ResultHandle = {};
                    if (EOS_SessionSearch_CopySearchResultByIndex(SearchHandle, &CopyOpts, &ResultHandle) !=
                        EOS_EResult::EOS_Success)
                    {
                        EOS_SessionSearch_Release(SearchHandle);
                        SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
                        UE_LOG(
                            LogRedpointEOS,
                            Error,
                            TEXT("FindSessions: Failed to copy search result at index %u"),
                            i);
                        This->TriggerOnFindSessionsCompleteDelegates(false);
                        return;
                    }

                    // Add the result.
                    SearchSettings->SearchResults.Add(
                        FOnlineSessionSearchResultEOS::CreateFromDetails(This->PlatformHandle, ResultHandle));
                }

                EOS_SessionSearch_Release(SearchHandle);
                SearchSettings->SearchState = EOnlineAsyncTaskState::Done;
                This->TriggerOnFindSessionsCompleteDelegates(true);
            }
        });
    return true;
}

bool FOnlineSessionInterfaceEOS::FindSessionById(
    const FUniqueNetId &SearchingUserId,
    const FUniqueNetId &SessionId,
    const FUniqueNetId &FriendId,
    const FOnSingleSessionResultCompleteDelegate &CompletionDelegate)
{
    if (SearchingUserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessionById: Searching user ID was invalid"));
        return false;
    }

    auto EOSSearchingUser = StaticCastSharedRef<const FUniqueNetIdEOS>(SearchingUserId.AsShared());
    if (!EOSSearchingUser->HasValidProductUserId())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessionById: Searching user ID was invalid"));
        return false;
    }

    if (SessionId.GetType() != REDPOINT_EOS_SUBSYSTEM_SESSION)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessionById: Session ID was invalid"));
        return false;
    }

    auto EOSSessionId = StaticCastSharedRef<const FUniqueNetIdEOSSession>(SessionId.AsShared());
    if (!EOSSessionId->IsValid())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessionById: Session ID was invalid"));
        return false;
    }

    EOS_Sessions_CreateSessionSearchOptions SearchOpts = {};
    SearchOpts.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
    SearchOpts.MaxSearchResults = 1;

    EOS_HSessionSearch SearchHandle = {};
    if (EOS_Sessions_CreateSessionSearch(this->PlatformHandle->Get<EOS_HSessions>(), &SearchOpts, &SearchHandle) !=
        EOS_EResult::EOS_Success)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessionById: Failed to create search handle"));
        return false;
    }

    auto SessionIdRaw = EOSString_SessionModification_SessionId::ToAnsiString(EOSSessionId->GetSessionId());

    EOS_SessionSearch_SetSessionIdOptions IdOpts = {};
    IdOpts.ApiVersion = EOS_SESSIONSEARCH_SETSESSIONID_API_LATEST;
    IdOpts.SessionId = SessionIdRaw.Ptr.Get();

    if (EOS_SessionSearch_SetSessionId(SearchHandle, &IdOpts) != EOS_EResult::EOS_Success)
    {
        EOS_SessionSearch_Release(SearchHandle);
        UE_LOG(LogRedpointEOS, Error, TEXT("FindSessionById: Failed to set session ID for search"));
        return false;
    }

    EOS_SessionSearch_FindOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
    Opts.LocalUserId = EOSSearchingUser->GetProductUserId();

    UE_LOG(
        LogRedpointEOS,
        Verbose,
        TEXT("FindSessionById: Searching for session ID '%s'..."),
        *EOSSessionId->GetSessionId());
    EOSRunOperation<EOS_HSessionSearch, EOS_SessionSearch_FindOptions, EOS_SessionSearch_FindCallbackInfo>(
        SearchHandle,
        &Opts,
        EOS_SessionSearch_Find,
        [WeakThis = GetWeakThis(this), CompletionDelegate, SearchHandle, EOSSearchingUser, EOSSessionId](
            const EOS_SessionSearch_FindCallbackInfo *Info) {
            if (auto This = PinWeakThis(WeakThis))
            {
                int32 LocalUserNum = 0;
                if (!This->Identity->GetLocalUserNum(*EOSSearchingUser, LocalUserNum))
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("FindSessionById: Searching user was not signed it at time of response"));
                    EOS_SessionSearch_Release(SearchHandle);
                    CompletionDelegate.ExecuteIfBound(
                        LocalUserNum,
                        false,
                        FOnlineSessionSearchResultEOS::CreateInvalid());
                    return;
                }

                if (Info->ResultCode != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("FindSessionById: Search failure: %s"),
                        *::ConvertError(Info->ResultCode).ToLogString());
                    EOS_SessionSearch_Release(SearchHandle);
                    CompletionDelegate.ExecuteIfBound(
                        LocalUserNum,
                        false,
                        FOnlineSessionSearchResultEOS::CreateInvalid());
                    return;
                }

                EOS_SessionSearch_GetSearchResultCountOptions CountOpts = {};
                CountOpts.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
                auto SearchCount = EOS_SessionSearch_GetSearchResultCount(SearchHandle, &CountOpts);

                UE_LOG(
                    LogRedpointEOS,
                    Verbose,
                    TEXT("FindSessionById: Got %d search results in response to search for a specific session ID."),
                    SearchCount,
                    *EOSSessionId->GetSessionId());

                if (SearchCount != 1)
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("FindSessionById: Unexpectedly got %d search results when searching for session ID."),
                        SearchCount);
                    EOS_SessionSearch_Release(SearchHandle);
                    CompletionDelegate.ExecuteIfBound(
                        LocalUserNum,
                        false,
                        FOnlineSessionSearchResultEOS::CreateInvalid());
                    return;
                }

                EOS_SessionSearch_CopySearchResultByIndexOptions CopyOpts = {};
                CopyOpts.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
                CopyOpts.SessionIndex = 0;

                EOS_HSessionDetails ResultHandle = {};
                if (EOS_SessionSearch_CopySearchResultByIndex(SearchHandle, &CopyOpts, &ResultHandle) !=
                    EOS_EResult::EOS_Success)
                {
                    UE_LOG(LogRedpointEOS, Error, TEXT("FindSessionById: Unable to copy search result."), SearchCount);
                    EOS_SessionSearch_Release(SearchHandle);
                    CompletionDelegate.ExecuteIfBound(
                        LocalUserNum,
                        false,
                        FOnlineSessionSearchResultEOS::CreateInvalid());
                    return;
                }

                UE_LOG(LogRedpointEOS, Verbose, TEXT("FindSessionById: Successfully got search result."));
                EOS_SessionSearch_Release(SearchHandle);
                CompletionDelegate.ExecuteIfBound(
                    LocalUserNum,
                    true,
                    FOnlineSessionSearchResultEOS::CreateFromDetails(This->PlatformHandle, ResultHandle));
            }
        });
    return true;
}

bool FOnlineSessionInterfaceEOS::CancelFindSessions()
{
    UE_LOG(LogRedpointEOS, Error, TEXT("CancelFindSessions not implemented."));
    return false;
}

bool FOnlineSessionInterfaceEOS::PingSearchResults(const FOnlineSessionSearchResult &SearchResult)
{
    UE_LOG(LogRedpointEOS, Error, TEXT("PingSearchResults not implemented."));
    return false;
}

bool FOnlineSessionInterfaceEOS::JoinSession(
    int32 LocalUserNum,
    FName SessionName,
    const FOnlineSessionSearchResult &DesiredSession)
{
    auto Id = this->Identity->GetUniquePlayerId(LocalUserNum);
    if (!Id.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("SearchingPlayerNum provided to FindSessions does not have online identity."));
        return false;
    }

    return this->JoinSession(Id.ToSharedRef().Get(), SessionName, DesiredSession);
}

bool FOnlineSessionInterfaceEOS::JoinSession(
    const FUniqueNetId &LocalUserId,
    FName SessionName,
    const FOnlineSessionSearchResult &DesiredSession)
{
    return this->JoinSession(LocalUserId, SessionName, DesiredSession, FOnJoinSessionCompleteDelegate());
}

bool FOnlineSessionInterfaceEOS::JoinSession(
    const FUniqueNetId &LocalUserId,
    FName SessionName,
    const FOnlineSessionSearchResult &DesiredSession,
    const FOnJoinSessionCompleteDelegate &OnComplete)
{
    if (!DesiredSession.Session.SessionInfo.IsValid())
    {
        auto Error = OnlineRedpointEOS::Errors::Sessions::InvalidSession(
            TEXT("FOnlineSessionInterfaceEOS::JoinSession"),
            TEXT("The DesiredSession parameter does not point to a valid session. If you got this session value from "
                 "FindSessionById, then the session was not found."));
        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Error.ToLogString());
        this->TriggerOnJoinSessionCompleteDelegates(SessionName, ConvertErrorTo_EOnJoinSessionCompleteResult(Error));
        OnComplete.ExecuteIfBound(SessionName, ConvertErrorTo_EOnJoinSessionCompleteResult(Error));
        return true;
    }

    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        Session = AddNamedSession(SessionName, DesiredSession.Session);
        Session->LocalOwnerId = LocalUserId.AsShared();
        Session->OwningUserId = nullptr;

        auto SessionNameAnsi = EOSString_SessionModification_SessionName::ToAnsiString(SessionName.ToString());

        auto EOSJoiningUser = StaticCastSharedRef<const FUniqueNetIdEOS>(LocalUserId.AsShared());
        if (!EOSJoiningUser->HasValidProductUserId())
        {
            RemoveNamedSession(SessionName);
            auto Error = OnlineRedpointEOS::Errors::InvalidUser(
                TEXT("FOnlineSessionInterfaceEOS::JoinSession"),
                TEXT("The local user ID was invalid."));
            UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Error.ToLogString());
            this->TriggerOnJoinSessionCompleteDelegates(
                SessionName,
                ConvertErrorTo_EOnJoinSessionCompleteResult(Error));
            OnComplete.ExecuteIfBound(SessionName, ConvertErrorTo_EOnJoinSessionCompleteResult(Error));
            return true;
        }

        auto SessionInfoEOS = StaticCastSharedPtr<FOnlineSessionInfoEOS>(DesiredSession.Session.SessionInfo);
        EOS_HSessionDetails Handle = SessionInfoEOS->GetHandle();

        EOS_Sessions_JoinSessionOptions Opts = {};
        Opts.ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
        Opts.SessionName = SessionNameAnsi.Ptr.Get();
        Opts.SessionHandle = Handle;
        Opts.LocalUserId = EOSJoiningUser->GetProductUserId();
        if (this->Config->GetPresenceAdvertisementType() == EPresenceAdvertisementType::Session)
        {
            // This setting is inherited from the host of the session.
            Opts.bPresenceEnabled = DesiredSession.Session.SessionSettings.bUsesPresence;
        }
        else
        {
            Opts.bPresenceEnabled = false;
        }

        FString GameServerAddr;
        TSharedPtr<FOnlineSessionInfoEOS> EOSInfo = StaticCastSharedPtr<FOnlineSessionInfoEOS>(Session->SessionInfo);
        if (!EOSInfo->GetResolvedConnectString(GameServerAddr, NAME_GamePort))
        {
            GameServerAddr = TEXT("");
        }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        auto SyntheticMaxPlayers = Session->SessionSettings.NumPublicConnections;
#endif

        EOSRunOperation<EOS_HSessions, EOS_Sessions_JoinSessionOptions, EOS_Sessions_JoinSessionCallbackInfo>(
            this->PlatformHandle->Get<EOS_HSessions>(),
            &Opts,
            EOS_Sessions_JoinSession,
            [WeakThis = GetWeakThis(this),
             SessionName,
             EOSJoiningUser,
             GameServerAddr,
             OnComplete
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
             ,
             SyntheticMaxPlayers
#endif
        ](const EOS_Sessions_JoinSessionCallbackInfo *Info) {
                if (auto This = PinWeakThis(WeakThis))
                {
                    FNamedOnlineSession *SessionInCallback = This->GetNamedSession(SessionName);
                    if (SessionInCallback == nullptr)
                    {
                        auto Error = ::ConvertError(
                            TEXT("FOnlineSessionInterfaceEOS::JoinSession"),
                            TEXT("Session was removed before JoinSession completed."),
                            Info->ResultCode);
                        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Error.ToLogString());
                        This->TriggerOnJoinSessionCompleteDelegates(
                            SessionName,
                            EOnJoinSessionCompleteResult::SessionDoesNotExist);
                        OnComplete.ExecuteIfBound(SessionName, EOnJoinSessionCompleteResult::SessionDoesNotExist);
                        return;
                    }

                    if (Info->ResultCode != EOS_EResult::EOS_Success)
                    {
                        This->RemoveNamedSession(SessionName);
                        auto Error = ::ConvertError(
                            TEXT("FOnlineSessionInterfaceEOS::JoinSession"),
                            TEXT("The local user ID was invalid."),
                            Info->ResultCode);
                        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Error.ToLogString());
                        This->TriggerOnJoinSessionCompleteDelegates(
                            SessionName,
                            ConvertErrorTo_EOnJoinSessionCompleteResult(Error));
                        OnComplete.ExecuteIfBound(SessionName, ConvertErrorTo_EOnJoinSessionCompleteResult(Error));
                        return;
                    }

                    auto SessionId = StaticCastSharedRef<const FUniqueNetIdEOSSession>(
                        SessionInCallback->SessionInfo->GetSessionId().AsShared());
                    auto SessionIdStr = SessionInCallback->GetSessionIdStr();

                    auto Finalize = [WeakThis = GetWeakThis(This),
                                     SessionName,
                                     SessionIdStr,
                                     OnComplete,
                                     EOSJoiningUser,
                                     GameServerAddr](const FOnlineError &Error) {
                        if (auto This = PinWeakThis(WeakThis))
                        {
                            if (!Error.bSucceeded)
                            {
                                UE_LOG(
                                    LogRedpointEOS,
                                    Warning,
                                    TEXT("JoinSession: One or more optional operations failed: %s"),
                                    *Error.ToLogString());
                            }

                            UE_LOG(
                                LogRedpointEOS,
                                Verbose,
                                TEXT("JoinSession: Successfully joined session '%s'"),
                                *SessionName.ToString());
                            This->TriggerOnJoinSessionCompleteDelegates(
                                SessionName,
                                EOnJoinSessionCompleteResult::Success);
                            OnComplete.ExecuteIfBound(SessionName, EOnJoinSessionCompleteResult::Success);
                            This->MetricsSend_BeginPlayerSession(*EOSJoiningUser, SessionIdStr, *GameServerAddr);
                        }
                    };

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
                    if (This->ShouldHaveSyntheticSession(SessionInCallback->SessionSettings))
                    {
                        auto SPM = This->PlatformHandle->GetSystem<FSyntheticSessionManager>();
                        SPM->UpsertSyntheticSession(
                            SessionId,
                            SyntheticMaxPlayers,
                            FSyntheticSessionManager::FOnComplete::CreateLambda([Finalize](const FError &Error) {
                                Finalize(AsOSSError(Error));
                            }));
                    }
                    else
                    {
                        Finalize(OnlineRedpointEOS::Errors::Success());
                    }
#else
                    Finalize(OnlineRedpointEOS::Errors::Success());
#endif

                    return;
                }
            });
        return true;
    }
    else
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("JoinSession: Failed because a session with the name %s already exists."),
            *SessionName.ToString());
        this->TriggerOnJoinSessionCompleteDelegates(SessionName, EOnJoinSessionCompleteResult::AlreadyInSession);
        OnComplete.ExecuteIfBound(SessionName, EOnJoinSessionCompleteResult::AlreadyInSession);
        return true;
    }
}

bool FOnlineSessionInterfaceEOS::FindFriendSession(int32 LocalUserNum, const FUniqueNetId &Friend)
{
    auto Id = this->Identity->GetUniquePlayerId(LocalUserNum);
    if (!Id.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("LocalUserNum provided to FindFriendSession does not have online identity."));
        return false;
    }

    return this->FindFriendSession(Id.ToSharedRef().Get(), Friend);
}

bool FOnlineSessionInterfaceEOS::FindFriendSession(const FUniqueNetId &LocalUserId, const FUniqueNetId &Friend)
{
    if (LocalUserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindFriendSession: Local user ID was invalid"));
        return false;
    }

    auto EOSSearchingUser = StaticCastSharedRef<const FUniqueNetIdEOS>(LocalUserId.AsShared());
    if (!EOSSearchingUser->HasValidProductUserId())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindFriendSession: Local user ID was invalid"));
        return false;
    }

    if (Friend.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindFriendSession: Friend user ID was invalid"));
        return false;
    }

    auto EOSTargetUser = StaticCastSharedRef<const FUniqueNetIdEOS>(Friend.AsShared());
    if (!EOSTargetUser->HasValidProductUserId())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindFriendSession: Friend user ID was invalid"));
        return false;
    }

    EOS_Sessions_CreateSessionSearchOptions SearchOpts = {};
    SearchOpts.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
    SearchOpts.MaxSearchResults = 1;

    EOS_HSessionSearch SearchHandle = {};
    if (EOS_Sessions_CreateSessionSearch(this->PlatformHandle->Get<EOS_HSessions>(), &SearchOpts, &SearchHandle) !=
        EOS_EResult::EOS_Success)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("FindFriendSession: Failed to create search handle"));
        return false;
    }

    EOS_SessionSearch_SetTargetUserIdOptions IdOpts = {};
    IdOpts.ApiVersion = EOS_SESSIONSEARCH_SETTARGETUSERID_API_LATEST;
    IdOpts.TargetUserId = EOSTargetUser->GetProductUserId();

    if (EOS_SessionSearch_SetTargetUserId(SearchHandle, &IdOpts) != EOS_EResult::EOS_Success)
    {
        EOS_SessionSearch_Release(SearchHandle);
        UE_LOG(LogRedpointEOS, Error, TEXT("FindFriendSession: Failed to set target user ID for search"));
        return false;
    }

    EOS_SessionSearch_FindOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
    Opts.LocalUserId = EOSSearchingUser->GetProductUserId();

    EOSRunOperation<EOS_HSessionSearch, EOS_SessionSearch_FindOptions, EOS_SessionSearch_FindCallbackInfo>(
        SearchHandle,
        &Opts,
        EOS_SessionSearch_Find,
        [WeakThis = GetWeakThis(this), SearchHandle, EOSSearchingUser](const EOS_SessionSearch_FindCallbackInfo *Info) {
            if (auto This = PinWeakThis(WeakThis))
            {
                int32 LocalUserNum = 0;
                if (!This->Identity->GetLocalUserNum(*EOSSearchingUser, LocalUserNum))
                {
                    UE_LOG(LogRedpointEOS, Error, TEXT("Searching user was not signed it at time of response"));
                    EOS_SessionSearch_Release(SearchHandle);
                    This->TriggerOnFindFriendSessionCompleteDelegates(
                        LocalUserNum,
                        false,
                        TArray<FOnlineSessionSearchResult>());
                    return;
                }

                EOS_SessionSearch_GetSearchResultCountOptions CountOpts = {};
                CountOpts.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;

                if (Info->ResultCode != EOS_EResult::EOS_Success ||
                    EOS_SessionSearch_GetSearchResultCount(SearchHandle, &CountOpts) != 1)
                {
                    EOS_SessionSearch_Release(SearchHandle);
                    This->TriggerOnFindFriendSessionCompleteDelegates(
                        LocalUserNum,
                        false,
                        TArray<FOnlineSessionSearchResult>());
                    return;
                }

                EOS_SessionSearch_CopySearchResultByIndexOptions CopyOpts = {};
                CopyOpts.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
                CopyOpts.SessionIndex = 0;

                EOS_HSessionDetails ResultHandle = {};
                if (EOS_SessionSearch_CopySearchResultByIndex(SearchHandle, &CopyOpts, &ResultHandle) !=
                    EOS_EResult::EOS_Success)
                {
                    EOS_SessionSearch_Release(SearchHandle);
                    This->TriggerOnFindFriendSessionCompleteDelegates(
                        LocalUserNum,
                        false,
                        TArray<FOnlineSessionSearchResult>());
                    return;
                }

                EOS_SessionSearch_Release(SearchHandle);

                TArray<FOnlineSessionSearchResult> Results;
                Results.Add(FOnlineSessionSearchResultEOS::CreateFromDetails(This->PlatformHandle, ResultHandle));
                This->TriggerOnFindFriendSessionCompleteDelegates(LocalUserNum, true, Results);
            }
        });
    return true;
}

bool FOnlineSessionInterfaceEOS::FindFriendSession(
    const FUniqueNetId &LocalUserId,
    const TArray<TSharedRef<const FUniqueNetId>> &FriendList)
{
    if (FriendList.Num() != 1)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("FindFriendSession with multiple friends is not supported. You must pass exactly one friend."));
        return false;
    }

    return this->FindFriendSession(LocalUserId, *FriendList[0]);
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FOnlineSessionInterfaceEOS::SendSessionInviteToRedpointEOSFriend(
    int32 LocalUserNum,
    TSharedRef<const FUniqueNetIdEOSSession> SessionId,
    TSharedRef<FOnlineFriend> FriendObj)
{
    FString RuntimePlatforms;
    if (!FriendObj->GetUserAttribute(TEXT("runtimePlatform.platformNames"), RuntimePlatforms))
    {
        UE_LOG(
            LogRedpointEOS,
            Verbose,
            TEXT("Friend with Redpoint EOS user ID did not supply 'runtimePlatform.platformNames' attribute, so we "
                 "can't send the invite over the local platform."));
        return false;
    }

    UE_LOG(LogRedpointEOS, Verbose, TEXT("Detected unified user info when sending session info to friend."));

    using namespace ::Redpoint::EOS::API::Common;
    using namespace ::Redpoint::EOS::Core::Utils;

    auto World = FWorldResolution::GetWorld(this->PlatformHandle->GetEnvironment()->InstanceName, true);
    if (!World.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("Game world is no longer valid at the point where we would send a local platform invite to a friend "
                 "for the target session, so we can't send the invite over the local platform."));
        return false;
    }

    TArray<FString> ExternalAccountTypes;
    RuntimePlatforms.ParseIntoArray(ExternalAccountTypes, TEXT(","));
    for (const auto &ExternalAccountTypeString : ExternalAccountTypes)
    {
        FName SubsystemName = NAME_None;
        auto ExternalAccountType = ExternalAccountTypeMap.Get(ExternalAccountTypeString);
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-default"
#pragma clang diagnostic ignored "-Wswitch"
#endif
        switch (ExternalAccountType)
        {
        case EOS_EExternalAccountType::EOS_EAT_STEAM:
            SubsystemName = STEAM_SUBSYSTEM;
            break;
        case EOS_EExternalAccountType::EOS_EAT_PSN:
        case EOS_EExternalAccountType::EOS_EAT_XBL:
        case EOS_EExternalAccountType::EOS_EAT_NINTENDO:
            SubsystemName = IOnlineSubsystem::GetByPlatform()->GetSubsystemName();
            break;
        }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

        if (SubsystemName.IsNone())
        {
            UE_LOG(
                LogRedpointEOS,
                Verbose,
                TEXT("There is no OSSv1 implementation for the runtime platform '%s'. This is normal and expected for "
                     "platforms that do not have a legacy OSSv1 implementation. Sending session invites to friends on "
                     "this runtime platform may be supported in a future release, depending on platform support."),
                *ExternalAccountTypeString);
            continue;
        }

        auto *Subsystem = Online::GetSubsystem(World.Get(), SubsystemName);
        if (Subsystem == nullptr)
        {
            UE_LOG(
                LogRedpointEOS,
                Verbose,
                TEXT("The Online::GetSubsystem call did not return an implementation for subsystem '%s'. This is "
                     "normal and expected if the game is not currently running on that platform or store."),
                *SubsystemName.ToString());
            continue;
        }

        auto NestedIdentity = Subsystem->GetIdentityInterface();
        if (!NestedIdentity.IsValid())
        {
            UE_LOG(
                LogRedpointEOS,
                Warning,
                TEXT("The OSSv1 subsystem implementation for '%s' did not return an implementation for "
                     "GetIdentityInterface, which is not expected."),
                *SubsystemName.ToString());
            continue;
        }

        auto TargetUserIdAttribute = FString::Printf(TEXT("runtimePlatform.friend.%s.id"), *ExternalAccountTypeString);
        FString TargetUserIdString;
        if (!FriendObj->GetUserAttribute(TargetUserIdAttribute, TargetUserIdString))
        {
            UE_LOG(
                LogRedpointEOS,
                Warning,
                TEXT("Friend with Redpoint EOS user ID did not supply '%s' attribute, even though the platform was "
                     "listed in the 'runtimePlatform.platformNames' attribute. This is a bug."),
                *TargetUserIdAttribute);
            continue;
        }

        auto TargetUserId = NestedIdentity->CreateUniquePlayerId(TargetUserIdString);
        if (!TargetUserId.IsValid())
        {
            UE_LOG(
                LogRedpointEOS,
                Warning,
                TEXT(
                    "The call to CreateUniquePlayerId to create an OSSv1 user ID from the string '%s' did not return a "
                    "value result for the '%s' online subsystem, so we can't send the invite over the local platform."),
                *TargetUserIdString,
                *SubsystemName.ToString());
            continue;
        }

        UE_LOG(
            LogRedpointEOS,
            Verbose,
            TEXT("Sending an invite to the session over the local platform to friend with local platform ID '%s'..."),
            *TargetUserId->ToString());
        auto SPM = this->PlatformHandle->GetSystem<FSyntheticSessionManager>();
        SPM->SendInvitationToSession(
            LocalUserNum,
            SessionId,
            TargetUserId.ToSharedRef(),
            FSyntheticSessionManager::FOnComplete());
        return true;
    }

    return false;
}

#endif

bool FOnlineSessionInterfaceEOS::SendSessionInviteToFriend(
    int32 LocalUserNum,
    FName SessionName,
    const FUniqueNetId &Friend)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto Id = this->Identity->GetUniquePlayerId(LocalUserNum);
    if (!Id.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("LocalUserNum provided to SendSessionInviteToFriend does not have online identity."));
        return false;
    }

    return this->SendSessionInviteToFriend(Id.ToSharedRef().Get(), SessionName, Friend);
#else
    UE_LOG(LogRedpointEOS, Error, TEXT("Sending session invites to friends is not supported on dedicated servers."));
    return false;
#endif
}

bool FOnlineSessionInterfaceEOS::SendSessionInviteToFriend(
    const FUniqueNetId &LocalUserId,
    FName SessionName,
    const FUniqueNetId &Friend)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (!this->Friends.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("Sending session invites to friends is not supported on dedicated servers."));
        return false;
    }

    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        FOnlineError Err = OnlineRedpointEOS::Errors::NotFound(
            LocalUserId,
            SessionName.ToString(),
            TEXT("FOnlineSessionInterfaceEOS::SendSessionInviteToFriend"),
            TEXT("There is not session with the specified local session name"));
        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Err.ToLogString());
        return false;
    }

    auto SessionId = StaticCastSharedRef<const FUniqueNetIdEOSSession>(Session->SessionInfo->GetSessionId().AsShared());

    if (LocalUserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        FOnlineError Err = OnlineRedpointEOS::Errors::InvalidUser(
            LocalUserId,
            SessionName.ToString(),
            TEXT("FOnlineSessionInterfaceEOS::SendSessionInviteToFriend"),
            TEXT("Specified local user was not an EOS user"));
        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Err.ToLogString());
        return false;
    }

    auto LocalUserIdRef = StaticCastSharedRef<const FUniqueNetIdEOS>(LocalUserId.AsShared());
    if (!LocalUserIdRef->HasValidProductUserId())
    {
        FOnlineError Err = OnlineRedpointEOS::Errors::InvalidUser(
            LocalUserId,
            SessionName.ToString(),
            TEXT("FOnlineSessionInterfaceEOS::SendSessionInviteToFriend"),
            TEXT("Specified local user was not an EOS user"));
        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Err.ToLogString());
        return false;
    }

    if (Friend.GetType().IsEqual(REDPOINT_EOS_SUBSYSTEM))
    {
        // Check to see if the target is a synthetic friend. If they are, send over the wrapped subsystems as well.
        int32 LocalUserNum;
        if (this->Identity->GetLocalUserNum(LocalUserId, LocalUserNum))
        {
            TSharedPtr<FOnlineFriend> FriendObj = this->Friends->GetFriend(LocalUserNum, Friend, TEXT(""));
            if (FriendObj.IsValid())
            {
                this->SendSessionInviteToRedpointEOSFriend(LocalUserNum, SessionId, FriendObj.ToSharedRef());
            }
        }

        // Now also send over EOS as well so that the game can receive a notification of the incoming invitation.
        auto RecipientIdRef = StaticCastSharedRef<const FUniqueNetIdEOS>(Friend.AsShared());
        if (!RecipientIdRef->HasValidProductUserId())
        {
            // We succeeded sending over any native subsystems though, so this is a success.
            return true;
        }

        // Send the invitation directly over EOS.
        EOS_Sessions_SendInviteOptions Opts = {};
        Opts.ApiVersion = EOS_SESSIONS_SENDINVITE_API_LATEST;
        EOSString_SessionModification_SessionName::AllocateToCharBuffer(SessionName.ToString(), Opts.SessionName);
        Opts.LocalUserId = LocalUserIdRef->GetProductUserId();
        Opts.TargetUserId = RecipientIdRef->GetProductUserId();

        EOSRunOperation<EOS_HSessions, EOS_Sessions_SendInviteOptions, EOS_Sessions_SendInviteCallbackInfo>(
            this->PlatformHandle->Get<EOS_HSessions>(),
            &Opts,
            *EOS_Sessions_SendInvite,
            [WeakThis = GetWeakThis(this), LocalUserIdRef, SessionName](
                const EOS_Sessions_SendInviteCallbackInfo *Info) {
                if (auto This = PinWeakThis(WeakThis))
                {
                    if (Info->ResultCode != EOS_EResult::EOS_Success)
                    {
                        FOnlineError Err = ::ConvertError(
                            *LocalUserIdRef,
                            SessionName.ToString(),
                            TEXT("FOnlineSessionInterfaceEOS::SendSessionInviteToFriend"),
                            TEXT("The EOS callback returned an error code, but we couldn't propagate this to the game "
                                 "code because the SendInvite API for sessions in Unreal Engine is synchronous (and "
                                 "the EOS API is asynchronous)."),
                            Info->ResultCode);
                        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Err.ToLogString());
                    }
                }
            });
        return true;
    }
    else
    {
        auto SPM = this->PlatformHandle->GetSystem<FSyntheticSessionManager>();
        int32 LocalUserNum;
        if (this->Identity->GetLocalUserNum(LocalUserId, LocalUserNum))
        {
            SPM->SendInvitationToSession(
                LocalUserNum,
                SessionId,
                Friend.AsShared(),
                FSyntheticSessionManager::FOnComplete::CreateLambda([](const FError &Error) {
                    if (!Error.WasSuccessful())
                    {
                        UE_LOG(LogRedpointEOS, Warning, TEXT("%s"), *Error.ToLogString());
                    }
                }));
            return true;
        }
        else
        {
            UE_LOG(
                LogRedpointEOS,
                Error,
                TEXT("SendSessionInviteToFriend: Unable to get local user ID when sending to native friend."));
        }

        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("SendSessionInviteToFriend: Target was native friend, but synthetic session manager was not "
                 "valid. Unable to send invitation at all."));
        return false;
    }
#else
    UE_LOG(LogRedpointEOS, Error, TEXT("Sending session invites to friends is not supported on dedicated servers."));
    return false;
#endif
}

bool FOnlineSessionInterfaceEOS::SendSessionInviteToFriends(
    int32 LocalUserNum,
    FName SessionName,
    const TArray<TSharedRef<const FUniqueNetId>> &FriendsArr)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto Id = this->Identity->GetUniquePlayerId(LocalUserNum);
    if (!Id.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("LocalUserNum provided to SendSessionInviteToFriend does not have online identity."));
        return false;
    }

    return this->SendSessionInviteToFriends(Id.ToSharedRef().Get(), SessionName, FriendsArr);
#else
    UE_LOG(LogRedpointEOS, Error, TEXT("Sending session invites to friends is not supported on dedicated servers."));
    return false;
#endif
}

bool FOnlineSessionInterfaceEOS::SendSessionInviteToFriends(
    const FUniqueNetId &LocalUserId,
    FName SessionName,
    const TArray<TSharedRef<const FUniqueNetId>> &FriendsArr)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (LocalUserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        FOnlineError Err = OnlineRedpointEOS::Errors::InvalidUser(
            LocalUserId,
            SessionName.ToString(),
            TEXT("FOnlineSessionInterfaceEOS::SendSessionInviteToFriends"),
            TEXT("Specified local user was not an EOS user"));
        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *Err.ToLogString());
        return true;
    }

    bool bAllSuccess = true;
    for (const auto &Friend : FriendsArr)
    {
        if (!this->SendSessionInviteToFriend(LocalUserId, SessionName, *Friend))
        {
            bAllSuccess = false;
        }
    }
    return bAllSuccess;
#else
    UE_LOG(LogRedpointEOS, Error, TEXT("Sending session invites to friends is not supported on dedicated servers."));
    return false;
#endif
}

bool FOnlineSessionInterfaceEOS::GetResolvedConnectString(FName SessionName, FString &ConnectInfo, FName PortType)
{
    FNamedOnlineSession *Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("GetResolvedConnectString: Session '%s' does not exist."),
            *SessionName.ToString());
        return false;
    }

    if (!Session->SessionInfo.IsValid())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("GetResolvedConnectString: Session does not have valid session info."));
        return false;
    }

    if (Session->SessionInfo->GetSessionId().GetType() != REDPOINT_EOS_SUBSYSTEM_SESSION)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("GetResolvedConnectString: Session is not from the EOS subsystem."));
        return false;
    }

    TSharedPtr<FOnlineSessionInfoEOS> EOSInfo = StaticCastSharedPtr<FOnlineSessionInfoEOS>(Session->SessionInfo);
    return EOSInfo->GetResolvedConnectString(ConnectInfo, PortType);
}

bool FOnlineSessionInterfaceEOS::GetResolvedConnectString(
    const class FOnlineSessionSearchResult &SearchResult,
    FName PortType,
    FString &ConnectInfo)
{
    if (!SearchResult.IsSessionInfoValid() || !SearchResult.Session.SessionInfo.IsValid())
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("GetResolvedConnectString: Search result does not have valid session info."));
        return false;
    }

    if (SearchResult.Session.SessionInfo->GetSessionId().GetType() != REDPOINT_EOS_SUBSYSTEM_SESSION)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("GetResolvedConnectString: Search result is not from the EOS subsystem."));
        return false;
    }

    TSharedPtr<FOnlineSessionInfoEOS> EOSInfo =
        StaticCastSharedPtr<FOnlineSessionInfoEOS>(SearchResult.Session.SessionInfo);
    return EOSInfo->GetResolvedConnectString(ConnectInfo, PortType);
}

FOnlineSessionSettings *FOnlineSessionInterfaceEOS::GetSessionSettings(FName SessionName)
{
    auto Session = this->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        return nullptr;
    }

    return &Session->SessionSettings;
}

bool FOnlineSessionInterfaceEOS::RegisterPlayer(FName SessionName, const FUniqueNetId &PlayerId, bool bWasInvited)
{
    TArray<TSharedRef<const FUniqueNetId>> Players;
    Players.Add(PlayerId.AsShared());
    return this->RegisterPlayers(SessionName, Players, bWasInvited);
}

bool FOnlineSessionInterfaceEOS::RegisterPlayers(
    FName SessionName,
    const TArray<TSharedRef<const FUniqueNetId>> &Players,
    bool bWasInvited)
{
    auto SessionNameAnsi = EOSString_SessionModification_SessionName::ToAnsiString(SessionName.ToString());

    for (const TSharedRef<const FUniqueNetId> &PlayerId : Players)
    {
        if (PlayerId->GetType() != REDPOINT_EOS_SUBSYSTEM)
        {
            UE_LOG(LogRedpointEOS, Error, TEXT("RegisterPlayers: Invalid player ID type."));
            return false;
        }
    }

    EOS_Sessions_RegisterPlayersOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONS_REGISTERPLAYERS_API_LATEST;
    Opts.SessionName = SessionNameAnsi.Ptr.Get();
    EOSString_ProductUserId::AllocateToIdListViaAccessor<TSharedRef<const FUniqueNetId>>(
        Players,
        [](const TSharedRef<const FUniqueNetId> &Player) {
            TSharedRef<const FUniqueNetIdEOS> PlayerIdEOS = StaticCastSharedRef<const FUniqueNetIdEOS>(Player);
            return PlayerIdEOS->GetProductUserId();
        },
        Opts.PlayersToRegisterCount,
        Opts.PlayersToRegister);

    EOSRunOperation<EOS_HSessions, EOS_Sessions_RegisterPlayersOptions, EOS_Sessions_RegisterPlayersCallbackInfo>(
        this->PlatformHandle->Get<EOS_HSessions>(),
        &Opts,
        EOS_Sessions_RegisterPlayers,
        [WeakThis = GetWeakThis(this), SessionName, Players, Opts](
            const EOS_Sessions_RegisterPlayersCallbackInfo *Info) {
            if (auto This = PinWeakThis(WeakThis))
            {
                EOSString_ProductUserId::FreeFromIdListConst(Opts.PlayersToRegisterCount, Opts.PlayersToRegister);

                if (Info->ResultCode == EOS_EResult::EOS_Success)
                {
                    FNamedOnlineSession *Session = This->GetNamedSession(SessionName);
                    if (Session != nullptr)
                    {
                        for (const TSharedRef<const FUniqueNetId> &PlayerId : Players)
                        {
                            Session->RegisteredPlayers.AddUnique(PlayerId);
                        }
                    }
                }
                else
                {
                    FString ErrorMessage = ::ConvertError(
                                               TEXT("FOnlineSessionInterfaceEOS::RegisterPlayers"),
                                               FString::Printf(
                                                   TEXT("Failed to register one or more players for session '%s'."),
                                                   *SessionName.ToString()),
                                               Info->ResultCode)
                                               .ToLogString();
                    if (SessionName.IsEqual(NAME_GameSession))
                    {
                        UE_LOG(LogRedpointEOS, Warning, TEXT("%s"), *ErrorMessage);
                    }
                    else
                    {
                        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *ErrorMessage);
                    }
                }

                This->TriggerOnRegisterPlayersCompleteDelegates(
                    SessionName,
                    Players,
                    Info->ResultCode == EOS_EResult::EOS_Success || Info->ResultCode == EOS_EResult::EOS_NoChange);
            }
        });
    return true;
}

bool FOnlineSessionInterfaceEOS::UnregisterPlayer(FName SessionName, const FUniqueNetId &PlayerId)
{
    TArray<TSharedRef<const FUniqueNetId>> Players;
    Players.Add(PlayerId.AsShared());
    return this->UnregisterPlayers(SessionName, Players);
}

bool FOnlineSessionInterfaceEOS::UnregisterPlayers(
    FName SessionName,
    const TArray<TSharedRef<const FUniqueNetId>> &Players)
{
    auto SessionNameAnsi = EOSString_SessionModification_SessionName::ToAnsiString(SessionName.ToString());

    for (const TSharedRef<const FUniqueNetId> &PlayerId : Players)
    {
        if (PlayerId->GetType() != REDPOINT_EOS_SUBSYSTEM)
        {
            UE_LOG(LogRedpointEOS, Error, TEXT("UnregisterPlayers: Invalid player ID type."));
            return false;
        }
    }

    EOS_Sessions_UnregisterPlayersOptions Opts = {};
    Opts.ApiVersion = EOS_SESSIONS_UNREGISTERPLAYERS_API_LATEST;
    Opts.SessionName = SessionNameAnsi.Ptr.Get();
    EOSString_ProductUserId::AllocateToIdListViaAccessor<TSharedRef<const FUniqueNetId>>(
        Players,
        [](const TSharedRef<const FUniqueNetId> &Player) {
            TSharedRef<const FUniqueNetIdEOS> PlayerIdEOS = StaticCastSharedRef<const FUniqueNetIdEOS>(Player);
            return PlayerIdEOS->GetProductUserId();
        },
        Opts.PlayersToUnregisterCount,
        Opts.PlayersToUnregister);

    EOSRunOperation<EOS_HSessions, EOS_Sessions_UnregisterPlayersOptions, EOS_Sessions_UnregisterPlayersCallbackInfo>(
        this->PlatformHandle->Get<EOS_HSessions>(),
        &Opts,
        EOS_Sessions_UnregisterPlayers,
        [WeakThis = GetWeakThis(this), SessionName, Players, Opts](
            const EOS_Sessions_UnregisterPlayersCallbackInfo *Info) {
            if (auto This = PinWeakThis(WeakThis))
            {
                EOSString_ProductUserId::FreeFromIdListConst(Opts.PlayersToUnregisterCount, Opts.PlayersToUnregister);

                if (Info->ResultCode == EOS_EResult::EOS_Success)
                {
                    FNamedOnlineSession *Session = This->GetNamedSession(SessionName);
                    if (Session != nullptr)
                    {
                        for (const TSharedRef<const FUniqueNetId> &PlayerId : Players)
                        {
                            Session->RegisteredPlayers.Remove(PlayerId);
                        }
                    }
                }
                else
                {
                    FString ErrorMessage = ::ConvertError(
                                               TEXT("FOnlineSessionInterfaceEOS::UnregisterPlayers"),
                                               FString::Printf(
                                                   TEXT("Failed to unregister one or more players for session '%s'."),
                                                   *SessionName.ToString()),
                                               Info->ResultCode)
                                               .ToLogString();
                    if (SessionName.IsEqual(NAME_GameSession))
                    {
                        UE_LOG(LogRedpointEOS, Warning, TEXT("%s"), *ErrorMessage);
                    }
                    else
                    {
                        UE_LOG(LogRedpointEOS, Error, TEXT("%s"), *ErrorMessage);
                    }
                }

                This->TriggerOnUnregisterPlayersCompleteDelegates(
                    SessionName,
                    Players,
                    Info->ResultCode == EOS_EResult::EOS_Success || Info->ResultCode == EOS_EResult::EOS_NoChange);
            }
        });
    return true;
}

void FOnlineSessionInterfaceEOS::RegisterLocalPlayer(
    const FUniqueNetId &PlayerId,
    FName SessionName,
    const FOnRegisterLocalPlayerCompleteDelegate &Delegate)
{
    // Not used.
    Delegate.ExecuteIfBound(PlayerId, EOnJoinSessionCompleteResult::Success);
}

void FOnlineSessionInterfaceEOS::UnregisterLocalPlayer(
    const FUniqueNetId &PlayerId,
    FName SessionName,
    const FOnUnregisterLocalPlayerCompleteDelegate &Delegate)
{
    // Not used.
    Delegate.ExecuteIfBound(PlayerId, true);
}

int32 FOnlineSessionInterfaceEOS::GetNumSessions()
{
    return this->Sessions.Num();
}

void FOnlineSessionInterfaceEOS::DumpSessionState()
{
}

}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

REDPOINT_EOS_CODE_GUARD_END()