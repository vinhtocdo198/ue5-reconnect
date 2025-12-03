// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformPresenceService.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Misc/Optional.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Caller/SafeOSSv1Caller.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Logging.h"
#include "RedpointEOSPlatformIntegrationOSSv1/WrappedExternalUserInfo.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1158129887, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Caller;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::UserCache;

FOSSv1RuntimePlatformPresenceService::FOSSv1RuntimePlatformPresenceService(
    const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
    const FName &InSubsystemName,
    const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver,
    const FString &InFriendsListName)
    : ExternalAccountTypeToHandle(InExternalAccountTypeToHandle)
    , SubsystemName(InSubsystemName)
    , PlatformStorage()
    , ExternalInfoResolver(InExternalInfoResolver)
    , FriendsListName(InFriendsListName)
{
}

void FOSSv1RuntimePlatformPresenceService::RegisterEvents()
{
    bool bIsGame = true;
#if WITH_EDITOR
    bIsGame = !GIsEditor;
#endif

    if (bIsGame)
    {
        auto *GlobalOSS = IOnlineSubsystem::Get(this->SubsystemName);
        if (GlobalOSS != nullptr)
        {
            auto SessionInterface = GlobalOSS->GetSessionInterface();
            if (SessionInterface.IsValid())
            {
                SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
                    FOnSessionUserInviteAcceptedDelegate::CreateSPLambda(
                        this,
                        [this](
                            const bool bWasSuccessful,
                            const int32,
                            FUniqueNetIdPtr,
                            const FOnlineSessionSearchResult &SearchResult) {
                            if (bWasSuccessful)
                            {
                                UE_LOG(
                                    LogRedpointEOSPlatformIntegrationOSSv1,
                                    Verbose,
                                    TEXT("Adding invite from global OSS '%s' to global queue."),
                                    *this->SubsystemName.ToString());
                                this->QueuedGlobalInvites.Add(SearchResult);
                            }
                        }));
            }
        }
    }
}

TOptional<bool> FOSSv1RuntimePlatformPresenceService::IsLocalUserOnline(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    return true;
}

void FOSSv1RuntimePlatformPresenceService::WaitForPlatformReadiness(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &InCompletionDelegate)
{
    // Some platforms require additional asynchronous work to be performed prior to each presence operation, and this
    // allows derived implementations of FOSSv1RuntimePlatformPresenceService to hook into this point.
    InCompletionDelegate.ExecuteIfBound();
}

void FOSSv1RuntimePlatformPresenceService::PlatformConfigureSearchForRoomId(
    FOnlineSessionSearch &Search,
    const FRoomIdRef &RoomId) const
{
    Search.MaxSearchResults = 1;
    Search.bIsLanQuery = false;
#if REDPOINT_EOS_UE_5_5_OR_LATER
    Search.QuerySettings.SearchParams.Add(SEARCH_LOBBIES, FOnlineSessionSearchParam(true, EOnlineComparisonOp::Equals));
#else
    Search.QuerySettings.SearchParams.Add(
        SEARCH_PRESENCE,
        FOnlineSessionSearchParam(true, EOnlineComparisonOp::Equals));
#endif
    Search.QuerySettings.SearchParams.Add(
        TEXT("RedpointEOSRoomId"),
        FOnlineSessionSearchParam(RoomId->ToString(), EOnlineComparisonOp::Equals));
}

void FOSSv1RuntimePlatformPresenceService::PlatformAssignRoomIdAndNamespaceToSettings(
    FOnlineSessionSettings &Settings,
    const FRoomIdRef &RoomId,
    const FName &RoomNamespace) const
{
    Settings.Settings.Add(
        TEXT("RedpointEOSRoomId"),
        FOnlineSessionSetting(RoomId->ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing));
    Settings.Settings.Add(
        TEXT("RedpointEOSRoomNamespace"),
        FOnlineSessionSetting(RoomNamespace.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing));
}

void FOSSv1RuntimePlatformPresenceService::PlatformTryGetRoomIdAndNamespaceFromSession(
    const FOnlineSession &Session,
    TOptional<FRoomIdRef> &OutRoomId,
    TOptional<FName> &OutNamespace) const
{
    const FOnlineSessionSetting *RoomIdPtr = Session.SessionSettings.Settings.Find(TEXT("RedpointEOSRoomId"));
    const FOnlineSessionSetting *RoomNamespacePtr =
        Session.SessionSettings.Settings.Find(TEXT("RedpointEOSRoomNamespace"));

    if (RoomIdPtr == nullptr || RoomIdPtr->Data.GetType() != EOnlineKeyValuePairDataType::String)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT(
                "Unable to find the 'RedpointEOSRoomId' attribute on the session invite that was received via the "
                "runtime platform integration, or the attribute had a non-string value. This invite will be ignored."));
        return;
    }
    if (RoomNamespacePtr == nullptr || RoomNamespacePtr->Data.GetType() != EOnlineKeyValuePairDataType::String)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Unable to find the 'RedpointEOSRoomNamespace' attribute on the session invite that was received via "
                 "the runtime platform integration, or the attribute had a non-string value. This invite will be "
                 "ignored."));
        return;
    }
    FString RedpointEOSRoomId, RedpointEOSRoomNamespace;
    RoomIdPtr->Data.GetValue(RedpointEOSRoomId);
    RoomNamespacePtr->Data.GetValue(RedpointEOSRoomNamespace);

    // Attempt to parse the room ID.
    auto RedpointEOSRoomIdParsed = FRoomId::FromString(RedpointEOSRoomId);
    if (!RedpointEOSRoomIdParsed.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("The 'RedpointEOSRoomId' attribute on the session invite could not be parsed to a room ID. The "
                 "original string value was '%s'. This invite will be ignored."),
            *RedpointEOSRoomId);
        return;
    }

    OutRoomId = RedpointEOSRoomIdParsed.ToSharedRef();
    OutNamespace = FName(*RedpointEOSRoomNamespace);
}

bool FOSSv1RuntimePlatformPresenceService::PlatformIsUnsuccessfulSearchStillOk() const
{
    return false;
}

bool FOSSv1RuntimePlatformPresenceService::PlatformSupportsSearch() const
{
    return true;
}

void FOSSv1RuntimePlatformPresenceService::UpdateLocalUserPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserState &InDesiredPresenceState,
    const FOnLocalUserPresenceUpdated &InCompletionDelegate)
{
    this->WaitForPlatformReadiness(
        CallContext,
        FSimpleDelegate::CreateSP(
            this,
            &FOSSv1RuntimePlatformPresenceService::UpdateLocalUserPresenceAfterPlatformReady,
            CallContext,
            InDesiredPresenceState,
            InCompletionDelegate));
}

void FOSSv1RuntimePlatformPresenceService::UpdateLocalUserPresenceAfterPlatformReady(
    FRuntimePlatformServiceCallContextRef CallContext,
    FPresenceUserState InDesiredPresenceState,
    FOnLocalUserPresenceUpdated InCompletionDelegate)
{
    // Get the target online subsystem, and the relevant interfaces.
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlinePresence> PresenceInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!PresenceInterface.IsValid())
            {
                PresenceInterface = OSSInstance->GetPresenceInterface();
            }
        }
    }
    // @note: We don't search for "Redpoint" prefixed online subsystems, because there is no Redpoint prefixed
    // implementation for presence.

    // If we still don't have an presence interface, this subsystem doesn't support presence.
    if (!PresenceInterface.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(TEXT("The subsystem '%s' does not support presence."), *this->SubsystemName.ToString())));
        return;
    }

    // Ask the target online subsystem to get the local user's FUniqueNetId for use with the presence interface.
    auto LocalUserId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
    if (!LocalUserId.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("Subsystem '%s' is not signed in for local user %d, so presence can not be updated on their "
                     "behalf."),
                *this->SubsystemName.ToString(),
                CallContext->LocalUserNum)));
        return;
    }

    // Cache the last desired presence so we can restore it later.
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    Storage.LastSetPresence.Add(
        CallContext->LocalUserNum,
        TTuple<FRuntimePlatformServiceCallContextRef, FPresenceUserState>(CallContext, InDesiredPresenceState));

    // Create our desired presence status for the online subsystem.
    auto RuntimePresence = MakeShared<FOnlineUserPresenceStatus>();
    if (!InDesiredPresenceState.bPresenceAdvertised)
    {
        RuntimePresence->State = EOnlinePresenceState::Offline;
    }
    else
    {
        switch (InDesiredPresenceState.ActivityState)
        {
        case EPresenceUserActivityState::Offline:
            RuntimePresence->State = EOnlinePresenceState::Offline;
            break;
        case EPresenceUserActivityState::DoNotDisturb:
            RuntimePresence->State = EOnlinePresenceState::DoNotDisturb;
            break;
        case EPresenceUserActivityState::Idle:
            RuntimePresence->State = EOnlinePresenceState::Away;
            break;
        case EPresenceUserActivityState::Online:
        default:
            RuntimePresence->State = EOnlinePresenceState::Online;
            break;
        }

        bool bRequiresLocalisedPresence = this->SubsystemName.IsEqual(STEAM_SUBSYSTEM);

        if (bRequiresLocalisedPresence)
        {
            auto TextId = FTextInspector::GetTextId(InDesiredPresenceState.StatusTextFormat.GetSourceText());
#if REDPOINT_EOS_UE_5_5_OR_LATER
            RuntimePresence->StatusStr =
                FString::Printf(TEXT("#%s_%s"), *TextId.GetNamespace().ToString(), *TextId.GetKey().ToString());
#else
            RuntimePresence->StatusStr =
                FString::Printf(TEXT("#%s_%s"), TextId.GetNamespace().GetChars(), TextId.GetKey().GetChars());
#endif
        }
        else
        {
            RuntimePresence->StatusStr = InDesiredPresenceState.GetStatusTextFormatted().ToString();
        }

        for (const auto &KV : InDesiredPresenceState.CustomAttributes)
        {
            switch (KV.Value.Type())
            {
            case FRoomAttribute::EType::String:
                RuntimePresence->Properties.Add(KV.Key, KV.Value.GetString());
                break;
            case FRoomAttribute::EType::Int64:
                RuntimePresence->Properties.Add(KV.Key, KV.Value.GetInt64());
                break;
            case FRoomAttribute::EType::Double:
                RuntimePresence->Properties.Add(KV.Key, KV.Value.GetDouble());
                break;
            case FRoomAttribute::EType::Bool:
                RuntimePresence->Properties.Add(KV.Key, KV.Value.GetBool());
                break;
            default:
                break;
            }
        }

        if (bRequiresLocalisedPresence)
        {
            for (const auto &KV : InDesiredPresenceState.StatusTextFormatArguments)
            {
                switch (KV.Value.GetType())
                {
                case EFormatArgumentType::Int:
                    RuntimePresence->Properties.Add(KV.Key, KV.Value.GetIntValue());
                    break;
                case EFormatArgumentType::UInt:
                    RuntimePresence->Properties.Add(KV.Key, KV.Value.GetUIntValue());
                    break;
                case EFormatArgumentType::Float:
                    RuntimePresence->Properties.Add(KV.Key, KV.Value.GetFloatValue());
                    break;
                case EFormatArgumentType::Double:
                    RuntimePresence->Properties.Add(KV.Key, KV.Value.GetDoubleValue());
                    break;
                case EFormatArgumentType::Text: {
                    auto TextId = FTextInspector::GetTextId(KV.Value.GetTextValue());
#if REDPOINT_EOS_UE_5_5_OR_LATER
                    RuntimePresence->Properties.Add(
                        KV.Key,
                        FString::Printf(
                            TEXT("#%s_%s"),
                            *TextId.GetNamespace().ToString(),
                            *TextId.GetKey().ToString()));
#else
                    RuntimePresence->Properties.Add(
                        KV.Key,
                        FString::Printf(TEXT("#%s_%s"), TextId.GetNamespace().GetChars(), TextId.GetKey().GetChars()));
#endif
                    break;
                }
                case EFormatArgumentType::Gender:
                    switch (KV.Value.GetGenderValue())
                    {
                    case ETextGender::Neuter:
                        RuntimePresence->Properties.Add(KV.Key, TEXT("N"));
                        break;
                    case ETextGender::Masculine:
                        RuntimePresence->Properties.Add(KV.Key, TEXT("M"));
                        break;
                    case ETextGender::Feminine:
                        RuntimePresence->Properties.Add(KV.Key, TEXT("F"));
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    // Now call the real presence implementation.
    PresenceInterface->SetPresence(
        *LocalUserId,
        *RuntimePresence,
        IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateSPLambda(
            this,
            [this, InCompletionDelegate](const FUniqueNetId &, const bool bWasSuccessful) {
                if (!bWasSuccessful)
                {
                    InCompletionDelegate.ExecuteIfBound(Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(
                            TEXT("The subsystem '%s' failed to set the user's presence, but the limitations of the "
                                 "online subsystem APIs prevent us from providing a more detailed error. Refer to "
                                 "other log entries to diagnose the cause."),
                            *this->SubsystemName.ToString())));
                }
                else
                {
                    InCompletionDelegate.ExecuteIfBound(Errors::Success());
                }
            }));
}

void FOSSv1RuntimePlatformPresenceService::UpsertRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
    bool bIsParty,
    const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate)
{
    this->UpsertRoomAdvertisementWithPoisonFlag(CallContext, InAdvertisedRoom, bIsParty, InCompletionDelegate, false);
}

void FOSSv1RuntimePlatformPresenceService::UpsertRoomAdvertisementWithPoisonFlag(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
    bool bIsParty,
    const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate,
    bool bExistingSessionIsPoisoned)
{
    this->WaitForPlatformReadiness(
        CallContext,
        FSimpleDelegate::CreateSP(
            this,
            &FOSSv1RuntimePlatformPresenceService::UpsertRoomAdvertisementWithPoisonFlagAfterPlatformReady,
            CallContext,
            InAdvertisedRoom,
            bIsParty,
            InCompletionDelegate,
            bExistingSessionIsPoisoned));
}

void FOSSv1RuntimePlatformPresenceService::UpsertRoomAdvertisementWithPoisonFlagAfterPlatformReady(
    FRuntimePlatformServiceCallContextRef CallContext,
    FPresenceUserAdvertisedRoom InAdvertisedRoom,
    bool bIsParty,
    FOnUpsertRoomAdvertisementComplete InCompletionDelegate,
    bool bExistingSessionIsPoisoned)
{
    auto PresenceAdvertises = CallContext->PlatformHandle->GetEnvironment()->Config->GetPresenceAdvertisementType();

    // OSS implementation can only advertise either party or session.
    if ((PresenceAdvertises != EPresenceAdvertisementType::Party && bIsParty) ||
        (PresenceAdvertises != EPresenceAdvertisementType::Session && !bIsParty))
    {
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    // Get the target online subsystem, and the relevant interfaces.
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineSession> SessionInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!SessionInterface.IsValid())
            {
                SessionInterface = OSSInstance->GetSessionInterface();
            }
        }
    }

    // If we still don't have an session interface, this subsystem doesn't support sessions.
    if (!SessionInterface.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("The subsystem '%s' does not support advertising presence via sessions."),
                *this->SubsystemName.ToString())));
        return;
    }

    // Ask the target online subsystem to get the local user's FUniqueNetId for use with the presence interface.
    auto LocalUserId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
    if (!LocalUserId.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("Subsystem '%s' is not signed in for local user %d, so presence can not be updated on their "
                     "behalf."),
                *this->SubsystemName.ToString(),
                CallContext->LocalUserNum)));
        return;
    }

    // Cache the last advertised room so we can restore it later.
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    Storage.LastAdvertisedRoom.Add(
        CallContext->LocalUserNum,
        TTuple<FRuntimePlatformServiceCallContextRef, TOptional<FPresenceUserAdvertisedRoom>, FRoomIdRef, bool>(
            CallContext,
            InAdvertisedRoom,
            InAdvertisedRoom.RoomId,
            bIsParty));

    // Create the search object so we can share it between our "session doesn't exist" and "session is poisoned"
    // branches.
    auto Search = MakeShared<FOnlineSessionSearch>();
    this->PlatformConfigureSearchForRoomId(*Search, InAdvertisedRoom.RoomId);

    // Check if we already have a session.
    FName SessionName = bIsParty ? NAME_PartySession : NAME_GameSession;
    FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(SessionName);
    if (ExistingSession == nullptr && this->PlatformSupportsSearch())
    {
        // No session, search for an existing one.
        FSafeOSSv1Caller::FindSessions(
            SessionInterface.ToSharedRef(),
            *LocalUserId,
            Search,
            FOnFindSessionsCompleteDelegate::CreateSP(
                this,
                &FOSSv1RuntimePlatformPresenceService::OnFindSessionsComplete,
                CallContext,
                InAdvertisedRoom,
                bIsParty,
                InCompletionDelegate,
                TWeakPtr<IOnlineSession>(SessionInterface),
                LocalUserId.ToSharedRef(),
                Search));
    }
    else if (ExistingSession == nullptr)
    {
        // No session, but the platform doesn't support searching. Just create one.
        auto NewSettings = MakeShared<FOnlineSessionSettings>();
        this->ApplyAdvertisedRoomToSessionSettings(InAdvertisedRoom, *NewSettings);
        FSafeOSSv1Caller::CreateSession(
            SessionInterface.ToSharedRef(),
            *LocalUserId,
            SessionName,
            *NewSettings,
            FOnCreateSessionCompleteDelegate::CreateSP(
                this,
                &FOSSv1RuntimePlatformPresenceService::OnCreateSessionComplete,
                CallContext,
                InAdvertisedRoom,
                bIsParty,
                InCompletionDelegate,
                TWeakPtr<IOnlineSession>(SessionInterface),
                SessionName));
    }
    else if (bExistingSessionIsPoisoned)
    {
        // Existing session, but it should be considered "no longer valid" because the Internet connection was lost and
        // we're currently trying to restore state.
        FSafeOSSv1Caller::DestroySession(
            SessionInterface.ToSharedRef(),
            SessionName,
            FOnDestroySessionCompleteDelegate::CreateSP(
                this,
                &FOSSv1RuntimePlatformPresenceService::OnDestroySessionCompletePriorToSearchComplete,
                CallContext,
                InAdvertisedRoom,
                bIsParty,
                InCompletionDelegate,
                TWeakPtr<IOnlineSession>(SessionInterface),
                LocalUserId.ToSharedRef(),
                Search));
    }
    else
    {
        // Existing session.
        auto NewSettings = ExistingSession->SessionSettings;
        this->ApplyAdvertisedRoomToSessionSettings(InAdvertisedRoom, NewSettings);
        FSafeOSSv1Caller::UpdateSession(
            SessionInterface.ToSharedRef(),
            SessionName,
            NewSettings,
            FOnUpdateSessionCompleteDelegate::CreateSP(
                this,
                &FOSSv1RuntimePlatformPresenceService::OnUpdateSessionComplete,
                CallContext,
                InAdvertisedRoom,
                bIsParty,
                InCompletionDelegate,
                TWeakPtr<IOnlineSession>(SessionInterface),
                SessionName));
    }
}

void FOSSv1RuntimePlatformPresenceService::OnDestroySessionCompletePriorToSearchComplete(
    FName SessionName,
    bool bWasSuccessful,
    FRuntimePlatformServiceCallContextRef CallContext,
    FPresenceUserAdvertisedRoom AdvertisedRoom,
    bool bIsParty,
    FOnUpsertRoomAdvertisementComplete CompletionDelegate,
    TWeakPtr<IOnlineSession> OnlineSessionWk,
    FUniqueNetIdRef LocalUserId,
    TSharedRef<FOnlineSessionSearch> Search)
{
    auto SessionInterface = OnlineSessionWk.Pin();
    if (!SessionInterface.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local platform's session service went away.")));
        return;
    }

    // We don't care if the DestroySession was successful; all we care about is whether the existing session is gone.
    if (SessionInterface->GetNamedSession(SessionName) != nullptr)
    {
        CompletionDelegate.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The local platform's session service still has an old session even though DestroySession was called. "
                 "This will prevent us from fixing the local user's presence information after loss of Internet "
                 "connectivity.")));
        return;
    }

    // Now start the search so we can go down the normal find/create logic.
    FSafeOSSv1Caller::FindSessions(
        SessionInterface.ToSharedRef(),
        *LocalUserId,
        Search,
        FOnFindSessionsCompleteDelegate::CreateSP(
            this,
            &FOSSv1RuntimePlatformPresenceService::OnFindSessionsComplete,
            CallContext,
            AdvertisedRoom,
            bIsParty,
            CompletionDelegate,
            TWeakPtr<IOnlineSession>(SessionInterface),
            LocalUserId,
            Search));
}

void FOSSv1RuntimePlatformPresenceService::DeleteRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &InRoomId,
    bool bIsParty,
    const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate)
{
    this->WaitForPlatformReadiness(
        CallContext,
        FSimpleDelegate::CreateSP(
            this,
            &FOSSv1RuntimePlatformPresenceService::DeleteRoomAdvertisementAfterPlatformReady,
            CallContext,
            InRoomId,
            bIsParty,
            InCompletionDelegate));
}

void FOSSv1RuntimePlatformPresenceService::DeleteRoomAdvertisementAfterPlatformReady(
    FRuntimePlatformServiceCallContextRef CallContext,
    FRoomIdRef InRoomId,
    bool bIsParty,
    FOnDeleteRoomAdvertisementComplete InCompletionDelegate)
{
    auto PresenceAdvertises = CallContext->PlatformHandle->GetEnvironment()->Config->GetPresenceAdvertisementType();

    // OSS implementation can only advertise either party or session.
    if ((PresenceAdvertises != EPresenceAdvertisementType::Party && bIsParty) ||
        (PresenceAdvertises != EPresenceAdvertisementType::Session && !bIsParty))
    {
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    // Get the target online subsystem, and the relevant interfaces.
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineSession> SessionInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!SessionInterface.IsValid())
            {
                SessionInterface = OSSInstance->GetSessionInterface();
            }
        }
    }

    // If we still don't have an session interface, this subsystem doesn't support sessions.
    if (!SessionInterface.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("The subsystem '%s' does not support advertising presence via sessions."),
                *this->SubsystemName.ToString())));
        return;
    }

    // Clear the last advertised room so we don't advertise it later.
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    Storage.LastAdvertisedRoom.Add(
        CallContext->LocalUserNum,
        TTuple<FRuntimePlatformServiceCallContextRef, TOptional<FPresenceUserAdvertisedRoom>, FRoomIdRef, bool>(
            CallContext,
            TOptional<FPresenceUserAdvertisedRoom>(),
            InRoomId,
            bIsParty));

    // Ask the target online subsystem to get the local user's FUniqueNetId for use with the presence interface.
    auto LocalUserId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
    if (!LocalUserId.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("Subsystem '%s' is not signed in for local user %d, so presence can not be updated on their "
                     "behalf."),
                *this->SubsystemName.ToString(),
                CallContext->LocalUserNum)));
        return;
    }

    // Attempt to the destroy session.
    FName SessionName = bIsParty ? NAME_PartySession : NAME_GameSession;
    FSafeOSSv1Caller::DestroySession(
        SessionInterface.ToSharedRef(),
        SessionName,
        FOnDestroySessionCompleteDelegate::CreateSP(
            this,
            &FOSSv1RuntimePlatformPresenceService::OnDestroySessionComplete,
            CallContext,
            InRoomId,
            bIsParty,
            InCompletionDelegate,
            TWeakPtr<IOnlineSession>(SessionInterface),
            SessionName));
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteReceived &FOSSv1RuntimePlatformPresenceService::
    OnRoomAdvertisementInviteReceived(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    return Storage.OnRoomAdvertisementInviteReceivedDelegate.FindOrAdd(CallContext->LocalUserNum);
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteAccepted &FOSSv1RuntimePlatformPresenceService::
    OnRoomAdvertisementInviteAccepted(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    return Storage.OnRoomAdvertisementInviteAcceptedDelegate.FindOrAdd(CallContext->LocalUserNum);
}

TOptional<EPresenceUserActivityState> FOSSv1RuntimePlatformPresenceService::GetNonUnifiedFriendPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId)
{
    // Get the target online subsystem, and the relevant interfaces.
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineFriends> FriendsInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!FriendsInterface.IsValid())
            {
                FriendsInterface = OSSInstance->GetFriendsInterface();
            }
        }
    }

    if (!IdentityInterface.IsValid() || !FriendsInterface.IsValid())
    {
        return TOptional<EPresenceUserActivityState>();
    }

    TArray<TSharedRef<FOnlineFriend>> Friends;
    FSafeOSSv1Caller::GetFriendsList(
        FriendsInterface.ToSharedRef(),
        CallContext->LocalUserNum,
        this->FriendsListName,
        Friends);
    for (const auto &Friend : Friends)
    {
        auto Result = ExternalInfoResolver(*Friend->GetUserId());
        if (Result.IsSet() && Result->AccountIdType == FriendAccountType && Result->AccountId == FriendAccountId)
        {
            switch (Friend->GetPresence().Status.State)
            {
            case EOnlinePresenceState::Online:
            case EOnlinePresenceState::Chat:
                return EPresenceUserActivityState::Online;
            case EOnlinePresenceState::Away:
            case EOnlinePresenceState::ExtendedAway:
                return EPresenceUserActivityState::Idle;
            case EOnlinePresenceState::DoNotDisturb:
                return EPresenceUserActivityState::DoNotDisturb;
            case EOnlinePresenceState::Offline:
            default:
                return EPresenceUserActivityState::Offline;
            }
        }
    }

    return TOptional<EPresenceUserActivityState>();
}

IRuntimePlatformPresenceService::FOnNonUnifiedFriendPresenceChanged &FOSSv1RuntimePlatformPresenceService::
    OnNonUnifiedFriendPresenceChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    return Storage.OnNonUnifiedFriendPresenceChangedDelegate.FindOrAdd(CallContext->LocalUserNum);
}

void FOSSv1RuntimePlatformPresenceService::SendRoomInviteToNonUnifiedFriend(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId,
    const FRoomIdRef &RoomId,
    const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate)
{
    // Get the target online subsystem, and the relevant interfaces.
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineFriends> FriendsInterface = nullptr;
    TSharedPtr<IOnlineSession> SessionInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!FriendsInterface.IsValid())
            {
                FriendsInterface = OSSInstance->GetFriendsInterface();
            }
            if (!SessionInterface.IsValid())
            {
                SessionInterface = OSSInstance->GetSessionInterface();
            }
        }
    }

    // If we still don't have the interfaces, this subsystem doesn't support sending invites.
    if (!IdentityInterface.IsValid() && !FriendsInterface.IsValid() && !SessionInterface.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("The subsystem '%s' does not support sending room invites."),
                *this->SubsystemName.ToString())));
        return;
    }

    // Ask the target online subsystem to get the local user's FUniqueNetId.
    auto LocalUserId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
    if (!LocalUserId.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("Subsystem '%s' is not signed in for local user %d, so room invites can not be sent over the "
                     "local platform."),
                *this->SubsystemName.ToString(),
                CallContext->LocalUserNum)));
        return;
    }

    // Find the local sessions to see if any of them have the target room ID.
    FName SelectedSessionName = NAME_None;
    auto *PartySession = SessionInterface->GetNamedSession(NAME_PartySession);
    auto *GameSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (PartySession != nullptr)
    {
        TOptional<FRoomIdRef> PartyRoomId;
        TOptional<FName> Unused;
        this->PlatformTryGetRoomIdAndNamespaceFromSession(*PartySession, PartyRoomId, Unused);
        if (PartyRoomId.IsSet() && PartyRoomId.GetValue()->IsEqual(RoomId))
        {
            SelectedSessionName = NAME_PartySession;
        }
    }
    if (SelectedSessionName.IsNone() && GameSession != nullptr)
    {
        TOptional<FRoomIdRef> GameRoomId;
        TOptional<FName> Unused;
        this->PlatformTryGetRoomIdAndNamespaceFromSession(*GameSession, GameRoomId, Unused);
        if (GameRoomId.IsSet() && GameRoomId.GetValue()->IsEqual(RoomId))
        {
            SelectedSessionName = NAME_GameSession;
        }
    }
    if (SelectedSessionName.IsNone())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::InvalidRequest(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The target room ID for sending the invite is not the room that the user is currently advertising as "
                 "a session or party, therefore a room invite can not be sent over the local platform.")));
        return;
    }

    // Find the friend so we can resolve the user ID to pass into the subsystem.
    TSharedPtr<const FUniqueNetId> TargetUserId;
    TArray<TSharedRef<FOnlineFriend>> Friends;
    FSafeOSSv1Caller::GetFriendsList(
        FriendsInterface.ToSharedRef(),
        CallContext->LocalUserNum,
        this->FriendsListName,
        Friends);
    for (const auto &Friend : Friends)
    {
        auto Result = ExternalInfoResolver(*Friend->GetUserId());
        if (Result.IsSet() && Result->AccountIdType == FriendAccountType && Result->AccountId == FriendAccountId)
        {
            TargetUserId = Friend->GetUserId();
            break;
        }
    }
    if (!TargetUserId.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::InvalidRequest(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The target user for sending the invite is not a friend of the user on the local platform, therefore "
                 "a room invite can not be sent over the local platform.")));
        return;
    }

    // Send the invite.
    if (!FSafeOSSv1Caller::SendSessionInviteToFriend(
            SessionInterface.ToSharedRef(),
            *LocalUserId,
            SelectedSessionName,
            *TargetUserId))
    {
        InCompletionDelegate.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The local platform was unable to send the room invite to the target user.")));
        return;
    }

    // We're done.
    InCompletionDelegate.ExecuteIfBound(Errors::Success());
}

bool FOSSv1RuntimePlatformPresenceService::TryOpenRoomInviteRecipientSelection(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &RoomId)
{
    // Get the target online subsystem, and the relevant interfaces.
    TSharedPtr<IOnlineSession> SessionInterface = nullptr;
    TSharedPtr<IOnlineExternalUI> ExternalUIInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!SessionInterface.IsValid())
            {
                SessionInterface = OSSInstance->GetSessionInterface();
            }
            if (!ExternalUIInterface.IsValid())
            {
                ExternalUIInterface = OSSInstance->GetExternalUIInterface();
            }
        }
    }

    // If we still don't have the interfaces, this subsystem doesn't support opening the overlay.
    if (!SessionInterface.IsValid() && !ExternalUIInterface.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("The subsystem '%s' does not support opening the overlay to send room invites."),
            *this->SubsystemName.ToString());
        return false;
    }

    // Find the local sessions to see if any of them have the target room ID.
    FName SelectedSessionName = NAME_None;
    auto *PartySession = SessionInterface->GetNamedSession(NAME_PartySession);
    auto *GameSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (PartySession != nullptr)
    {
        TOptional<FRoomIdRef> PartyRoomId;
        TOptional<FName> Unused;
        this->PlatformTryGetRoomIdAndNamespaceFromSession(*PartySession, PartyRoomId, Unused);
        if (PartyRoomId.IsSet() && PartyRoomId.GetValue()->IsEqual(RoomId))
        {
            SelectedSessionName = NAME_PartySession;
        }
    }
    if (SelectedSessionName.IsNone() && GameSession != nullptr)
    {
        TOptional<FRoomIdRef> GameRoomId;
        TOptional<FName> Unused;
        this->PlatformTryGetRoomIdAndNamespaceFromSession(*GameSession, GameRoomId, Unused);
        if (GameRoomId.IsSet() && GameRoomId.GetValue()->IsEqual(RoomId))
        {
            SelectedSessionName = NAME_GameSession;
        }
    }
    if (SelectedSessionName.IsNone())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("The target room ID for sending the invite is not the room that the user is currently advertising as "
                 "a session or party, therefore a room invite can not be sent over the local platform."));
        return false;
    }

    // Open the external UI.
    return FSafeOSSv1Caller::ShowInviteUI(
        ExternalUIInterface.ToSharedRef(),
        CallContext->LocalUserNum,
        SelectedSessionName);
}

void FOSSv1RuntimePlatformPresenceService::RegisterEventsBeforeLogin(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &InCompletionDelegate)
{
    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        VeryVerbose,
        TEXT("Registering events for subsystem '%s'."),
        *this->SubsystemName.ToString());

    // Get the target online subsystem, and the relevant interfaces.
    IOnlineSubsystem *OSSInstance = nullptr;
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineSession> SessionInterface = nullptr;
    TSharedPtr<IOnlinePresence> PresenceInterface = nullptr;
    {
        OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!SessionInterface.IsValid())
            {
                SessionInterface = OSSInstance->GetSessionInterface();
            }
            if (!PresenceInterface.IsValid())
            {
                PresenceInterface = OSSInstance->GetPresenceInterface();
            }
        }
    }

    // If we still don't have an session interface, this subsystem doesn't support sessions.
    if (!SessionInterface.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            VeryVerbose,
            TEXT("The subsystem '%s' does not support advertising presence via sessions."),
            *this->SubsystemName.ToString());
        InCompletionDelegate.ExecuteIfBound();
        return;
    }

    // Get the handle storage specific to this platform instance.
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];

    // Register events on that session interface for when we receive invites. Note that we don't currently unregister
    // these events anywhere, since we expect both this runtime platform integration and session implementation to last
    // for the lifetime of the game (i.e. no play-in-editor support).
    Storage.OnSessionInviteReceivedDelegateHandle.Add(
        CallContext->LocalUserNum,
        SessionInterface->AddOnSessionInviteReceivedDelegate_Handle(FOnSessionInviteReceivedDelegate::CreateSP(
            this,
            &FOSSv1RuntimePlatformPresenceService::OnSessionInviteReceived,
            CallContext)));
    Storage.OnSessionInviteAcceptedDelegateHandle.Add(
        CallContext->LocalUserNum,
        SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateSP(
            this,
            &FOSSv1RuntimePlatformPresenceService::OnSessionInviteAccepted,
            CallContext)));

    if (PresenceInterface.IsValid() && !Storage.OnPresenceReceivedDelegateHandle.IsValid())
    {
        Storage.OnPresenceReceivedDelegateHandle =
            PresenceInterface->AddOnPresenceReceivedDelegate_Handle(FOnPresenceReceivedDelegate::CreateSP(
                this,
                &FOSSv1RuntimePlatformPresenceService::OnPresenceReceived,
                CallContext));
    }

    if (!Storage.OnConnectivityStateChangedDelegateHandle.IsValid())
    {
        Storage.OnConnectivityStateChangedDelegateHandle =
            OSSInstance->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateSP(
                this,
                &FOSSv1RuntimePlatformPresenceService::OnConnectionStatusChanged,
                CallContext));
    }

    InCompletionDelegate.ExecuteIfBound();
}

void FOSSv1RuntimePlatformPresenceService::RestoreInvitesAfterLogin(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &InCompletionDelegate)
{
    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        VeryVerbose,
        TEXT("Restoring invites for subsystem '%s'."),
        *this->SubsystemName.ToString());

    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    Storage.LastSetPresence.Remove(CallContext->LocalUserNum);
    Storage.LastAdvertisedRoom.Remove(CallContext->LocalUserNum);

    // Fire any events from the global queue.
    auto InvitesToFire = this->QueuedGlobalInvites;
    this->QueuedGlobalInvites.Empty();

    for (const auto &Invite : InvitesToFire)
    {
        this->OnSessionInviteAccepted(true, CallContext->LocalUserNum, nullptr, Invite, CallContext);
    }

    InCompletionDelegate.ExecuteIfBound();
}

void FOSSv1RuntimePlatformPresenceService::UnregisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        VeryVerbose,
        TEXT("Unregistering events for subsystem '%s'."),
        *this->SubsystemName.ToString());

    // Get the target online subsystem, and the relevant interfaces.
    IOnlineSubsystem *OSSInstance = nullptr;
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineSession> SessionInterface = nullptr;
    TSharedPtr<IOnlinePresence> PresenceInterface = nullptr;
    {
        OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!SessionInterface.IsValid())
            {
                SessionInterface = OSSInstance->GetSessionInterface();
            }
            if (!PresenceInterface.IsValid())
            {
                PresenceInterface = OSSInstance->GetPresenceInterface();
            }
        }
    }

    // If we still don't have an session interface, this subsystem doesn't support sessions.
    if (!SessionInterface.IsValid())
    {
        return;
    }

    // Get the handle storage specific to this platform instance.
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];

    // Unregister all set events.
    {
        auto *Handle = Storage.OnSessionInviteReceivedDelegateHandle.Find(CallContext->LocalUserNum);
        if (Handle != nullptr)
        {
            SessionInterface->ClearOnSessionInviteReceivedDelegate_Handle(*Handle);
        }
    }
    {
        auto *Handle = Storage.OnSessionInviteAcceptedDelegateHandle.Find(CallContext->LocalUserNum);
        if (Handle != nullptr)
        {
            SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(*Handle);
        }
    }
    if (PresenceInterface.IsValid() && Storage.OnPresenceReceivedDelegateHandle.IsValid())
    {
        PresenceInterface->ClearOnPresenceReceivedDelegate_Handle(Storage.OnPresenceReceivedDelegateHandle);
        Storage.OnPresenceReceivedDelegateHandle.Reset();
    }
    if (Storage.OnConnectivityStateChangedDelegateHandle.IsValid())
    {
        OSSInstance->ClearOnConnectionStatusChangedDelegate_Handle(Storage.OnConnectivityStateChangedDelegateHandle);
        Storage.OnConnectivityStateChangedDelegateHandle.Reset();
    }
}

void FOSSv1RuntimePlatformPresenceService::ApplyAdvertisedRoomToSessionSettings(
    const FPresenceUserAdvertisedRoom &AdvertisedRoom,
    FOnlineSessionSettings &Settings)
{
    Settings.NumPublicConnections = static_cast<int32>(AdvertisedRoom.MaximumMemberCount);
    Settings.NumPrivateConnections = 0;
    Settings.bShouldAdvertise = true;
    Settings.bAllowJoinInProgress = true; // @todo
    Settings.bIsLANMatch = false;
    Settings.bIsDedicated = false;
    Settings.bUsesStats = false;
    Settings.bAllowInvites = true;
    Settings.bUsesPresence = true;
    Settings.bAllowJoinViaPresence = AdvertisedRoom.bJoinable;
    Settings.bAllowJoinViaPresenceFriendsOnly = false;
    Settings.bAntiCheatProtected = false;
    Settings.bUseLobbiesIfAvailable = true;
    Settings.bUseLobbiesVoiceChatIfAvailable = false;
    Settings.SessionIdOverride = AdvertisedRoom.RoomId->ToString();
    Settings.BuildUniqueId = GetBuildUniqueId();
    this->PlatformAssignRoomIdAndNamespaceToSettings(Settings, AdvertisedRoom.RoomId, AdvertisedRoom.RoomNamespace);
}

void FOSSv1RuntimePlatformPresenceService::OnFindSessionsComplete(
    bool bWasSuccessful,
    FRuntimePlatformServiceCallContextRef CallContext,
    FPresenceUserAdvertisedRoom AdvertisedRoom,
    bool bIsParty,
    FOnUpsertRoomAdvertisementComplete CompletionDelegate,
    TWeakPtr<IOnlineSession> OnlineSessionWk,
    FUniqueNetIdRef LocalUserId,
    TSharedRef<FOnlineSessionSearch> Search)
{
    auto SessionInterface = OnlineSessionWk.Pin();
    if (!SessionInterface.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local platform's session service went away.")));
        return;
    }

    if (!bWasSuccessful && !this->PlatformIsUnsuccessfulSearchStillOk())
    {
        CompletionDelegate.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The local platform's session service returned an error, but the design of the online subsystem APIs "
                 "prevents us from determining the exact cause. Refer to other logs for the reason.")));
        return;
    }

    FName SessionName = bIsParty ? NAME_PartySession : NAME_GameSession;

    // We now need to filter search results by actually matching the room ID attribute. This is because some platforms
    // can only obtain approximate, multiple results and further client-side filtering is required.
    TOptional<FOnlineSessionSearchResult> FilteredSearchResult;
    for (const auto &SearchResult : Search->SearchResults)
    {
        TOptional<FRoomIdRef> ResultRoomId;
        TOptional<FName> Unused;
        this->PlatformTryGetRoomIdAndNamespaceFromSession(SearchResult.Session, ResultRoomId, Unused);
        if (ResultRoomId.IsSet() && ResultRoomId.GetValue()->IsEqual(AdvertisedRoom.RoomId))
        {
            FilteredSearchResult = SearchResult;
            break;
        }
    }

    if (!FilteredSearchResult.IsSet())
    {
        auto NewSettings = MakeShared<FOnlineSessionSettings>();
        this->ApplyAdvertisedRoomToSessionSettings(AdvertisedRoom, *NewSettings);
        FSafeOSSv1Caller::CreateSession(
            SessionInterface.ToSharedRef(),
            *LocalUserId,
            SessionName,
            *NewSettings,
            FOnCreateSessionCompleteDelegate::CreateSP(
                this,
                &FOSSv1RuntimePlatformPresenceService::OnCreateSessionComplete,
                CallContext,
                AdvertisedRoom,
                bIsParty,
                CompletionDelegate,
                TWeakPtr<IOnlineSession>(SessionInterface),
                SessionName));
    }
    else
    {
        // Some online subsystems, such as Steam, can incorrectly deserialize the bUseLobbiesIfAvailable and
        // bUsesPresence settings from search results. Since we know we created all sessions with presence and lobbies
        // enabled, forcibly update the values in the search result so that the join will succeed.
        auto ModifiedSearchResult = FilteredSearchResult.GetValue();
        ModifiedSearchResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;
        ModifiedSearchResult.Session.SessionSettings.bUsesPresence = true;

        FSafeOSSv1Caller::JoinSession(
            SessionInterface.ToSharedRef(),
            *LocalUserId,
            SessionName,
            ModifiedSearchResult,
            FOnJoinSessionCompleteDelegate::CreateSP(
                this,
                &FOSSv1RuntimePlatformPresenceService::OnJoinSessionComplete,
                CallContext,
                AdvertisedRoom,
                bIsParty,
                CompletionDelegate,
                TWeakPtr<IOnlineSession>(SessionInterface),
                SessionName));
    }
}

void FOSSv1RuntimePlatformPresenceService::OnCreateSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    FRuntimePlatformServiceCallContextRef CallContext,
    FPresenceUserAdvertisedRoom AdvertisedRoom,
    bool bIsParty,
    FOnUpsertRoomAdvertisementComplete CompletionDelegate,
    TWeakPtr<IOnlineSession> OnlineSessionWk,
    FName ExpectedSessionName)
{
    auto SessionInterface = OnlineSessionWk.Pin();
    if (!SessionInterface.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local platform's session service went away.")));
        return;
    }

    if (!bWasSuccessful)
    {
        CompletionDelegate.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The local platform's session service returned an error, but the design of the online subsystem APIs "
                 "prevents us from determining the exact cause. Refer to other logs for the reason.")));
        return;
    }

    CompletionDelegate.ExecuteIfBound(Errors::Success());
}

void FOSSv1RuntimePlatformPresenceService::OnJoinSessionComplete(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type ResultType,
    FRuntimePlatformServiceCallContextRef CallContext,
    FPresenceUserAdvertisedRoom AdvertisedRoom,
    bool bIsParty,
    FOnUpsertRoomAdvertisementComplete CompletionDelegate,
    TWeakPtr<IOnlineSession> OnlineSessionWk,
    FName ExpectedSessionName)
{
    auto SessionInterface = OnlineSessionWk.Pin();
    if (!SessionInterface.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local platform's session service went away.")));
        return;
    }

    if (ResultType != EOnJoinSessionCompleteResult::Success)
    {
        switch (ResultType)
        {
        case EOnJoinSessionCompleteResult::AlreadyInSession:
            CompletionDelegate.ExecuteIfBound(Errors::Sessions::NotAllowed(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local platform's session service returned an error while attempting to join the room "
                     "advertisement.")));
            break;
        case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
            CompletionDelegate.ExecuteIfBound(Errors::Sessions::InvalidSession(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local platform's session service returned an error while attempting to join the room "
                     "advertisement.")));
            break;
        case EOnJoinSessionCompleteResult::SessionDoesNotExist:
            CompletionDelegate.ExecuteIfBound(Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local platform's session service returned an error while attempting to join the room "
                     "advertisement.")));
            break;
        case EOnJoinSessionCompleteResult::SessionIsFull:
            CompletionDelegate.ExecuteIfBound(Errors::Sessions::HostAtCapacity(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local platform's session service returned an error while attempting to join the room "
                     "advertisement.")));
            break;
        case EOnJoinSessionCompleteResult::UnknownError:
        default:
            CompletionDelegate.ExecuteIfBound(Errors::UnexpectedError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local platform's session service returned an error, but the design of the online subsystem "
                     "APIs prevents us from determining the exact cause. Refer to other logs for the reason.")));
            break;
        }
        return;
    }

    CompletionDelegate.ExecuteIfBound(Errors::Success());
}

void FOSSv1RuntimePlatformPresenceService::OnUpdateSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    FRuntimePlatformServiceCallContextRef CallContext,
    FPresenceUserAdvertisedRoom AdvertisedRoom,
    bool bIsParty,
    FOnUpsertRoomAdvertisementComplete CompletionDelegate,
    TWeakPtr<IOnlineSession> OnlineSessionWk,
    FName ExpectedSessionName)
{
    auto SessionInterface = OnlineSessionWk.Pin();
    if (!SessionInterface.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local platform's session service went away.")));
        return;
    }

    if (!bWasSuccessful)
    {
        CompletionDelegate.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The local platform's session service returned an error, but the design of the online subsystem APIs "
                 "prevents us from determining the exact cause. Refer to other logs for the reason.")));
        return;
    }

    CompletionDelegate.ExecuteIfBound(Errors::Success());
}

void FOSSv1RuntimePlatformPresenceService::OnDestroySessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    FRuntimePlatformServiceCallContextRef CallContext,
    FRoomIdRef RoomId,
    bool bIsParty,
    FOnUpsertRoomAdvertisementComplete CompletionDelegate,
    TWeakPtr<IOnlineSession> OnlineSessionWk,
    FName ExpectedSessionName)
{
    auto SessionInterface = OnlineSessionWk.Pin();
    if (!SessionInterface.IsValid())
    {
        CompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local platform's session service went away.")));
        return;
    }

    if (!bWasSuccessful)
    {
        CompletionDelegate.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The local platform's session service returned an error, but the design of the online subsystem APIs "
                 "prevents us from determining the exact cause. Refer to other logs for the reason.")));
        return;
    }

    CompletionDelegate.ExecuteIfBound(Errors::Success());
}

void FOSSv1RuntimePlatformPresenceService::OnSessionInviteReceived(
    const FUniqueNetId &UserId,
    const FUniqueNetId &FromId,
    const FString &AppId,
    const FOnlineSessionSearchResult &InviteResult,
    FRuntimePlatformServiceCallContextRef CallContext)
{
    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        Verbose,
        TEXT("Received invite to room via presence advertisement on '%s' subsystem."),
        *this->SubsystemName.ToString());

    // Get the identity system and look up the signed in user for the current local user num.
    auto IdentitySystem = CallContext->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(CallContext->LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("%s"),
            *Errors::NoConnection(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 FString::Printf(
                     TEXT("The local user %d isn't currently signed into Epic Online Services, so the invite from the "
                          "online subsystem can't be received."),
                     CallContext->LocalUserNum))
                 .ToLogString());
        return;
    }

    // Attempt to extract the room ID from the search result.
    TOptional<FRoomIdRef> RedpointEOSRoomId;
    TOptional<FName> RedpointEOSRoomNamespace;
    this->PlatformTryGetRoomIdAndNamespaceFromSession(
        InviteResult.Session,
        RedpointEOSRoomId,
        RedpointEOSRoomNamespace);
    if (!RedpointEOSRoomId.IsSet() || !RedpointEOSRoomNamespace.IsSet())
    {
        return;
    }

    // Look up the sending user's user info using their external user ID. It's not possible for a user to have a room
    // advertisement on a runtime platform without having signed in before, and if they've signed in, then there will be
    // a link from their external account ID back to the PUID.
    auto UserCache = CallContext->PlatformHandle->GetSystem<IUserCacheSystem>();
    UserCache->GetUsersByExternalAccountIds(
        IdentityUser->GetUserId(),
        this->ExternalAccountTypeToHandle,
        TArray<FString>{FromId.ToString()},
        IUserCacheSystem::FOnGetUsersByExternalAccountIdsComplete::CreateSPLambda(
            this,
            [this,
             FromIdStr = FromId.ToString(),
             CallContext,
             IdentityUser,
             RedpointEOSRoomId,
             RedpointEOSRoomNamespace](FError CallErrorCode, TMap<FString, FErrorableUserInfo> UserInfos) {
                if (!CallErrorCode.WasSuccessful() || UserInfos[FromIdStr].IsErrored())
                {
                    UE_LOG(
                        LogRedpointEOSPlatformIntegrationOSSv1,
                        Error,
                        TEXT("The sender of the session invite from the runtime platform integration could not be "
                             "resolved back to an EOS PUID. The user ID that was searched for was '%s'."),
                        *FromIdStr);
                    return;
                }

                this->OnRoomAdvertisementInviteReceived(CallContext)
                    .Broadcast(
                        IdentityUser->GetUserId(),
                        UserInfos[FromIdStr].GetUserInfo(),
                        RedpointEOSRoomId.GetValue(),
                        RedpointEOSRoomNamespace.GetValue());
            }));

    // @note: We don't need to handle any joining of the room advertisement, since once the room is actually joined on
    // EOS, the user will join it via the UpsertRoomAdvertisement code path.
}

void FOSSv1RuntimePlatformPresenceService::OnSessionInviteAccepted(
    const bool bWasSuccessful,
    const int32 ControllerId,
    FUniqueNetIdPtr,
    const FOnlineSessionSearchResult &InviteResult,
    FRuntimePlatformServiceCallContextRef CallContext)
{
    if (!bWasSuccessful)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("The runtime platform integration received an OnSessionUserInviteAccepted event from the underlying "
                 "online subsystem, but bWasSuccessful was false so this event and invite will be ignored."));
        return;
    }

    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        Verbose,
        TEXT("Accepted invite to room via presence advertisement on '%s' subsystem."),
        *this->SubsystemName.ToString());

    // Get the identity system and look up the signed in user for the current local user num.
    auto IdentitySystem = CallContext->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(CallContext->LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("%s"),
            *Errors::NoConnection(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 FString::Printf(
                     TEXT("The local user %d isn't currently signed into Epic Online Services, so the invite from the "
                          "online subsystem can't be received."),
                     CallContext->LocalUserNum))
                 .ToLogString());
        return;
    }

    // Attempt to extract the room ID from the search result.
    TOptional<FRoomIdRef> RedpointEOSRoomId;
    TOptional<FName> RedpointEOSRoomNamespace;
    this->PlatformTryGetRoomIdAndNamespaceFromSession(
        InviteResult.Session,
        RedpointEOSRoomId,
        RedpointEOSRoomNamespace);
    if (!RedpointEOSRoomId.IsSet() || !RedpointEOSRoomNamespace.IsSet())
    {
        return;
    }

    this->OnRoomAdvertisementInviteAccepted(CallContext)
        .Broadcast(
            IdentityUser->GetUserId(),
            nullptr,
            RedpointEOSRoomId.GetValue(),
            RedpointEOSRoomNamespace.GetValue());

    // @note: We don't need to handle any joining of the room advertisement, since once the room is actually joined on
    // EOS, the user will join it via the UpsertRoomAdvertisement code path.
}

void FOSSv1RuntimePlatformPresenceService::OnPresenceReceived(
    const FUniqueNetId &UserId,
    const TSharedRef<FOnlineUserPresence> &Presence,
    FRuntimePlatformServiceCallContextRef CallContext)
{
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];

    // Get the target online subsystem, and the relevant interfaces.
    TSharedPtr<IOnlineFriends> FriendsInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!FriendsInterface.IsValid())
            {
                FriendsInterface = OSSInstance->GetFriendsInterface();
            }
        }
    }

    if (!FriendsInterface.IsValid())
    {
        return;
    }

    auto Friend = FriendsInterface->GetFriend(CallContext->LocalUserNum, UserId, TEXT(""));
    if (!Friend.IsValid())
    {
        return;
    }

    auto Result = this->ExternalInfoResolver(*Friend->GetUserId());
    if (Result.IsSet())
    {
        auto ActivityState = EPresenceUserActivityState::Offline;
        switch (Friend->GetPresence().Status.State)
        {
        case EOnlinePresenceState::Online:
        case EOnlinePresenceState::Chat:
            ActivityState = EPresenceUserActivityState::Online;
            break;
        case EOnlinePresenceState::Away:
        case EOnlinePresenceState::ExtendedAway:
            ActivityState = EPresenceUserActivityState::Idle;
            break;
        case EOnlinePresenceState::DoNotDisturb:
            ActivityState = EPresenceUserActivityState::DoNotDisturb;
            break;
        case EOnlinePresenceState::Offline:
        default:
            ActivityState = EPresenceUserActivityState::Offline;
            break;
        }

        Storage.OnNonUnifiedFriendPresenceChangedDelegate.FindOrAdd(CallContext->LocalUserNum)
            .Broadcast(CallContext->LocalUserNum, Result->AccountIdType, Result->AccountId, ActivityState);
    }
}

void FOSSv1RuntimePlatformPresenceService::OnConnectionStatusChanged(
    const FString &ServiceName,
    EOnlineServerConnectionStatus::Type LastConnectionState,
    EOnlineServerConnectionStatus::Type ConnectionState,
    FRuntimePlatformServiceCallContextRef CallContext)
{
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];

    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        Verbose,
        TEXT("Online subsystem '%s' reported connection state is changing from '%s' (real: '%s') to '%s'."),
        *this->SubsystemName.ToString(),
        EOnlineServerConnectionStatus::ToString(LastConnectionState),
        EOnlineServerConnectionStatus::ToString(Storage.RealLastConnectionState),
        EOnlineServerConnectionStatus::ToString(ConnectionState));

    if (((LastConnectionState != EOnlineServerConnectionStatus::Normal &&
          LastConnectionState != EOnlineServerConnectionStatus::Connected) ||
         (Storage.RealLastConnectionState != EOnlineServerConnectionStatus::Normal &&
          Storage.RealLastConnectionState != EOnlineServerConnectionStatus::Connected)) &&
        (ConnectionState == EOnlineServerConnectionStatus::Normal ||
         ConnectionState == EOnlineServerConnectionStatus::Connected))
    {
        // The OSSv1 reports we have now reconnected to the Internet. Re-apply presence for all users that have them.

        // @note: The invocation of UpdateLocalUserPresence and UpsertRoomAdvertisement will update our maps (to insert
        // new cache values). We must make a copy of the maps and iterate on those so that the iteration doesn't break
        // due to the underlying map changing.
        auto LastSetPresenceCopy = Storage.LastSetPresence;
        auto LastAdvertisedRoomCopy = Storage.LastAdvertisedRoom;

        for (const auto &KV : LastSetPresenceCopy)
        {
            UE_LOG(
                LogRedpointEOSPlatformIntegrationOSSv1,
                Verbose,
                TEXT("Restoring runtime platform presence for local user %d of online subsystem '%s' after Internet "
                     "connectivity restored."),
                KV.Key,
                *this->SubsystemName.ToString());
            this->UpdateLocalUserPresence(
                KV.Value.Get<0>(),
                KV.Value.Get<1>(),
                FOnLocalUserPresenceUpdated::CreateStatic(
                    &FOSSv1RuntimePlatformPresenceService::OnPresenceRestoredAfterConnectivityRestored,
                    KV.Key,
                    this->SubsystemName));
        }
        for (const auto &KV : LastAdvertisedRoomCopy)
        {
            if (KV.Value.Get<1>().IsSet())
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationOSSv1,
                    Verbose,
                    TEXT("Restoring runtime platform advertised room for local user %d of online subsystem '%s' after "
                         "Internet connectivity restored."),
                    KV.Key,
                    *this->SubsystemName.ToString());
                this->UpsertRoomAdvertisementWithPoisonFlag(
                    KV.Value.Get<0>(),
                    KV.Value.Get<1>().GetValue(),
                    KV.Value.Get<3>(),
                    FOnUpsertRoomAdvertisementComplete::CreateStatic(
                        &FOSSv1RuntimePlatformPresenceService::OnRoomAdvertisementRestoredAfterConnectivityRestored,
                        KV.Key,
                        this->SubsystemName),
                    true);
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationOSSv1,
                    Verbose,
                    TEXT("Clearing runtime platform advertised room for local user %d of online subsystem '%s' after "
                         "Internet connectivity restored."),
                    KV.Key,
                    *this->SubsystemName.ToString());
                this->DeleteRoomAdvertisement(
                    KV.Value.Get<0>(),
                    KV.Value.Get<2>(),
                    KV.Value.Get<3>(),
                    FOnDeleteRoomAdvertisementComplete::CreateStatic(
                        &FOSSv1RuntimePlatformPresenceService::OnRoomAdvertisementClearedAfterConnectivityRestored,
                        KV.Key,
                        this->SubsystemName));
            }
        }
    }

    Storage.RealLastConnectionState = ConnectionState;
}

void FOSSv1RuntimePlatformPresenceService::OnPresenceRestoredAfterConnectivityRestored(
    FError ErrorCode,
    int32 LocalUserNum,
    FName SubsystemNameCaptured)
{
    if (ErrorCode.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Verbose,
            TEXT("Successfully restored runtime platform presence for local user %d of "
                 "online subsystem '%s' after Internet connectivity restored."),
            LocalUserNum,
            *SubsystemNameCaptured.ToString());
    }
    else
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Failed to restore runtime platform presence for local user %d of online "
                 "subsystem '%s' after Internet connectivity restored: %s"),
            LocalUserNum,
            *SubsystemNameCaptured.ToString(),
            *ErrorCode.ToLogString());
    }
}

void FOSSv1RuntimePlatformPresenceService::OnRoomAdvertisementRestoredAfterConnectivityRestored(
    FError ErrorCode,
    int32 LocalUserNum,
    FName SubsystemNameCaptured)
{
    if (ErrorCode.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Verbose,
            TEXT("Successfully restored runtime platform advertised room for local user %d of "
                 "online "
                 "subsystem '%s' after Internet connectivity restored."),
            LocalUserNum,
            *SubsystemNameCaptured.ToString());
    }
    else
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Failed to restore runtime platform advertised room for local user %d of online "
                 "subsystem '%s' after Internet connectivity restored: %s"),
            LocalUserNum,
            *SubsystemNameCaptured.ToString(),
            *ErrorCode.ToLogString());
    }
}

void FOSSv1RuntimePlatformPresenceService::OnRoomAdvertisementClearedAfterConnectivityRestored(
    FError ErrorCode,
    int32 LocalUserNum,
    FName SubsystemNameCaptured)
{
    if (ErrorCode.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Verbose,
            TEXT("Successfully cleared runtime platform advertised room for local user %d of "
                 "online subsystem '%s' after Internet connectivity restored."),
            LocalUserNum,
            *SubsystemNameCaptured.ToString());
    }
    else
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Failed to clear runtime platform advertised room for local user %d of online "
                 "subsystem '%s' after Internet connectivity restored: %s"),
            LocalUserNum,
            *SubsystemNameCaptured.ToString(),
            *ErrorCode.ToLogString());
    }
}

};

#endif

REDPOINT_EOS_CODE_GUARD_END()