// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/Services/MetaRuntimePlatformPresenceService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "Dom/JsonObject.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatformIntegrationMeta/Logging.h"
#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"
#include "RedpointEOSPlatformIntegrationMeta/Services/MetaExternalUserInfo.h"
#include "RedpointEOSPresence/PresenceUserActivityState.h"
#include "RedpointEOSPresence/PresenceUserAdvertisedRoom.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3098476363, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Integration::Meta;
using namespace ::Redpoint::EOS::Platform::Integration::Meta::Services;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::UserCache;

FMetaRuntimePlatformPresenceService::FMetaRuntimePlatformPresenceService()
    : OnRoomAdvertisementInviteReceivedDelegate()
    , OnRoomAdvertisementInviteAcceptedDelegate()
    , OnNonUnifiedFriendPresenceChangedDelegate()
    , RefreshFriendPresenceTimer(MakeShared<FPeriodicTimer>(10.0f /* Refresh every 10 seconds. */))
    , FriendPresenceState()
    , PlatformHandle()
    , DeeplinkBuffer()
    , CurrentlyAdvertisedParty()
    , CurrentlyAdvertisedSession()
{
}

void FMetaRuntimePlatformPresenceService::RegisterEvents()
{
    this->RefreshFriendPresenceTimer->Start(
        FPeriodicTimerDelegate::CreateSP(this, &FMetaRuntimePlatformPresenceService::RefreshFriendPresence));
    FMetaSDK::OnDeeplinkReceived().AddSP(this, &FMetaRuntimePlatformPresenceService::OnDeeplinkReceived);
}

void FMetaRuntimePlatformPresenceService::SetPlatformHandle(const FPlatformHandle &InPlatformHandle)
{
    this->PlatformHandle = InPlatformHandle;

    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(0);
    if (!IdentityUser.IsValid())
    {
        // Not ready to process deeplinks yet.
        return;
    }

    auto BufferList = this->DeeplinkBuffer;
    this->DeeplinkBuffer.Empty();
    for (const auto &Deeplink : BufferList)
    {
        this->OnDeeplinkReceived(Deeplink);
    }
}

void FMetaRuntimePlatformPresenceService::FlushDeeplinkEventsAfterLogin()
{
    auto BufferList = this->DeeplinkBuffer;
    this->DeeplinkBuffer.Empty();
    for (const auto &Deeplink : BufferList)
    {
        this->OnDeeplinkReceived(Deeplink);
    }
}

void FMetaRuntimePlatformPresenceService::RefreshFriendPresence()
{
    FMetaSDK::GetLoggedInUserFriends(FMetaSDK::FOnGetLoggedInUserFriendsComplete::CreateSP(
        this,
        &FMetaRuntimePlatformPresenceService::OnGetLoggedInUserFriendsComplete));
}

void FMetaRuntimePlatformPresenceService::OnGetLoggedInUserFriendsComplete(
    const FError &ErrorCode,
    const TOptional<TArray<ovrUserHandle>> &Friends)
{
    if (!ErrorCode.WasSuccessful() || !Friends.IsSet())
    {
        // This is a periodic refresh; if it fails, we don't care.
        return;
    }

    // Remove any friends that are no longer on the friends list.
    TSet<ovrID> ReturnedFriends;
    for (const auto &Friend : Friends.GetValue())
    {
        ReturnedFriends.Add(ovr_User_GetID(Friend));
    }
    TSet<ovrID> RemovedFriends;
    for (const auto &FriendId : ReturnedFriends)
    {
        if (!this->FriendPresenceState.Contains(FriendId))
        {
            RemovedFriends.Add(FriendId);
        }
    }
    for (const auto &RemovedFriend : RemovedFriends)
    {
        this->FriendPresenceState.Remove(RemovedFriend);
    }

    // Upsert the presence of any returned friends.
    for (const auto &Friend : Friends.GetValue())
    {
        auto FriendId = ovr_User_GetID(Friend);
        auto FriendPresenceStatus = ovr_User_GetPresenceStatus(Friend);

        auto FriendIdStr = FMetaSDK::UserIdToString(FriendId);
        if (!FriendIdStr.IsSet())
        {
            continue;
        }

        auto *ExistingPresence = this->FriendPresenceState.Find(FriendId);
        EPresenceUserActivityState NewPresence;

        switch (FriendPresenceStatus)
        {
        case ovrUserPresenceStatus_Online:
            NewPresence = EPresenceUserActivityState::Online;
            break;
        case ovrUserPresenceStatus_Offline:
        case ovrUserPresenceStatus_Unknown:
        default:
            NewPresence = EPresenceUserActivityState::Offline;
            break;
        }

        bool bFireEvent = ExistingPresence == nullptr || *ExistingPresence != NewPresence;
        this->FriendPresenceState.Add(FriendId, NewPresence);
        if (bFireEvent)
        {
            this->OnNonUnifiedFriendPresenceChangedDelegate
                .Broadcast(0, EOS_EExternalAccountType::EOS_EAT_OCULUS, FriendIdStr.GetValue(), NewPresence);
        }
    }
}

void FMetaRuntimePlatformPresenceService::OnDeeplinkReceived(const FString &Deeplink)
{
    if (!this->PlatformHandle.IsValid())
    {
        this->DeeplinkBuffer.Add(Deeplink);
        return;
    }

    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(0);
    if (!IdentityUser.IsValid())
    {
        this->DeeplinkBuffer.Add(Deeplink);
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Deeplink);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Error,
            TEXT("Deeplink '%s' is being dropped because it is invalid."),
            *Deeplink);
        return;
    }

    {
        FString RoomIdString, RoomNamespaceString;
        if (JsonObject->TryGetStringField(TEXT("PartyRoomId"), RoomIdString) &&
            JsonObject->TryGetStringField(TEXT("PartyRoomNamespace"), RoomNamespaceString))
        {
            auto RoomIdTmp = FRoomId::FromString(RoomIdString);
            if (RoomIdTmp.IsValid())
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationMeta,
                    Verbose,
                    TEXT("Deeplink '%s' is broadcasting a room advertisement invite accept for a party."),
                    *Deeplink);
                this->OnRoomAdvertisementInviteAcceptedDelegate.Broadcast(
                    IdentityUser->GetUserId(),
                    nullptr,
                    RoomIdTmp.ToSharedRef(),
                    FName(*RoomNamespaceString));
            }
        }
    }

    {
        FString RoomIdString, RoomNamespaceString;
        if (JsonObject->TryGetStringField(TEXT("SessionRoomId"), RoomIdString) &&
            JsonObject->TryGetStringField(TEXT("SessionRoomNamespace"), RoomNamespaceString))
        {
            auto RoomIdTmp = FRoomId::FromString(RoomIdString);
            if (RoomIdTmp.IsValid())
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationMeta,
                    Verbose,
                    TEXT("Deeplink '%s' is broadcasting a room advertisement invite accept for a session. The game is "
                         "expected to handle this and join the game server."),
                    *Deeplink);
                this->OnRoomAdvertisementInviteAcceptedDelegate.Broadcast(
                    IdentityUser->GetUserId(),
                    nullptr,
                    RoomIdTmp.ToSharedRef(),
                    FName(*RoomNamespaceString));
            }
        }
    }
}

TOptional<bool> FMetaRuntimePlatformPresenceService::IsLocalUserOnline(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    // There is no Meta SDK API available to get the local user's current online status (technically there is
    // ovr_User_GetPresenceStatus, but it requires an asynchronous query so is unlikely to automatically refresh based
    // on what the local user changes in their settings).
    return TOptional<bool>();
}

void FMetaRuntimePlatformPresenceService::UpdateLocalUserPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserState &InDesiredPresenceState,
    const FOnLocalUserPresenceUpdated &InCompletionDelegate)
{
    // The Meta SDK does not support setting presence status (either enumeration or text values). The only thing it
    // supports is advertising rooms. Thus, this operation always succeeds.
    InCompletionDelegate.ExecuteIfBound(Errors::Success());
}

void FMetaRuntimePlatformPresenceService::UpsertRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
    bool bIsParty,
    const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate)
{
#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SET
    if (bIsParty)
    {
        this->CurrentlyAdvertisedParty = InAdvertisedRoom;
    }
    else
    {
        this->CurrentlyAdvertisedSession = InAdvertisedRoom;
    }

    this->SyncRoomAdvertisement(
        CallContext,
        FOnUpsertRoomAdvertisementComplete::CreateLambda([InCompletionDelegate](const FError &ErrorCode) {
            InCompletionDelegate.ExecuteIfBound(ErrorCode);
        }));
#else
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("This game is built against a version of the Meta Quest SDK that is too old to support presence "
             "operations. Please upgrade the Oculus Platform SDK or Unreal Engine to at least 5.5 for presence to "
             "work.")));
    return;
#endif
}

void FMetaRuntimePlatformPresenceService::DeleteRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &InRoomId,
    bool bIsParty,
    const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate)
{
#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SET
    if (bIsParty)
    {
        this->CurrentlyAdvertisedParty.Reset();
    }
    else
    {
        this->CurrentlyAdvertisedSession.Reset();
    }

    this->SyncRoomAdvertisement(
        CallContext,
        FOnUpsertRoomAdvertisementComplete::CreateLambda([InCompletionDelegate](const FError &ErrorCode) {
            InCompletionDelegate.ExecuteIfBound(ErrorCode);
        }));
#else
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("This game is built against a version of the Meta Quest SDK that is too old to support presence "
             "operations. Please upgrade the Oculus Platform SDK or Unreal Engine to at least 5.5 for presence to "
             "work.")));
    return;
#endif
}

#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SET

void FMetaRuntimePlatformPresenceService::SyncRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate)
{
    // Initialize the SDK if needed.
    if (!FMetaSDK::InitIfNeeded())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The Meta Quest SDK is not initialized, so presence can not be set.")));
        return;
    }

    // Locate the current world. Meta Quest's APIs are all orientated around "destinations", which are expected to
    // virtual spaces that a user can join into. In Unreal Engine, the most logical thing this correlates with is a map
    // or server, rather than a party or session. The current map name (e.g. MyMapName) will be used as the destination
    // API name.
    if (!CallContext->World.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::InvalidRequest(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("There is no current world, so Meta Quest presence can not be updated.")));
        return;
    }

    // Create the group presence options so we can start initializing it's settings.
    auto GroupPresenceOptions = ovr_GroupPresenceOptions_Create();

    // Create the deeplink JSON object, which will tell joining players where to go (potentially joining both a party
    // and session at the same time).
    auto JsonObject = MakeShared<FJsonObject>();

    // Set the destination API as the current map name.
    UE_LOG(
        LogRedpointEOSPlatformIntegrationMeta,
        Verbose,
        TEXT("The destination API name that will be used for group presence is: %s"),
        *CallContext->World->GetMapName());
    ovr_GroupPresenceOptions_SetDestinationApiName(
        GroupPresenceOptions,
        TCHAR_TO_ANSI(*CallContext->World->GetMapName()));

    // If we're in a single player map OR we don't have an advertised session, we advertise the map name as the
    // destination API and, if the player has a joinable party, advertise that as the target. When players join, they'll
    // join the party but they won't be on the same map.
    if (!IsValid(CallContext->World->GetNetDriver()) || !this->CurrentlyAdvertisedSession.IsSet())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Verbose,
            TEXT("The game is currently in a single player map, or does not have an advertised multiplayer session. "
                 "Friends will only be able to join the party if there is one, and when they do so, they will end up "
                 "on their own copy of the map (not in multiplayer)."));

        // Advertise a party to join if we have one.
        if (this->CurrentlyAdvertisedParty.IsSet())
        {
            UE_LOG(
                LogRedpointEOSPlatformIntegrationMeta,
                Verbose,
                TEXT("Advertising the party '%s' on Meta Quest with bJoinable '%s'."),
                *this->CurrentlyAdvertisedParty->RoomId->ToString(),
                this->CurrentlyAdvertisedParty->bJoinable ? TEXT("true") : TEXT("false"));
            ovr_GroupPresenceOptions_SetLobbySessionId(
                GroupPresenceOptions,
                TCHAR_TO_ANSI(*this->CurrentlyAdvertisedParty->RoomId->ToString()));
            ovr_GroupPresenceOptions_SetIsJoinable(GroupPresenceOptions, this->CurrentlyAdvertisedParty->bJoinable);

            if (this->CurrentlyAdvertisedParty->bJoinable)
            {
                JsonObject->SetStringField(TEXT("PartyRoomId"), this->CurrentlyAdvertisedParty->RoomId->ToString());
                JsonObject->SetStringField(
                    TEXT("PartyRoomNamespace"),
                    this->CurrentlyAdvertisedParty->RoomNamespace.ToString());
            }
        }
        // Otherwise we're not joinable.
        else
        {
            UE_LOG(
                LogRedpointEOSPlatformIntegrationMeta,
                Verbose,
                TEXT("There is no party to advertise, so the user will not be joinable."));
            ovr_GroupPresenceOptions_SetIsJoinable(GroupPresenceOptions, false);
        }
    }
    // Otherwise, we're on a multiplayer map and we have a session ID to advertise. If the user has a party to
    // advertise, the party will be set as the lobby ID and the session will be set as the match. Otherwise, the session
    // will be set as the lobby ID because group presence *requires* a lobby ID to be set in order for players to be
    // joinable.
    //
    // If a party and session are advertised, when players join they'll join both.
    //
    // If only a session is advertised, when players join they'll join the game server.
    else
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Verbose,
            TEXT("The game is currently in on a multiplayer map with an advertised multiplayer session."));

        // Advertise party + session if we have both.
        if (this->CurrentlyAdvertisedParty.IsSet())
        {
            UE_LOG(
                LogRedpointEOSPlatformIntegrationMeta,
                Verbose,
                TEXT("Advertising the party '%s' and session '%s' on Meta Quest with bJoinable '%s'."),
                *this->CurrentlyAdvertisedParty->RoomId->ToString(),
                *this->CurrentlyAdvertisedSession->RoomId->ToString(),
                (this->CurrentlyAdvertisedParty->bJoinable || this->CurrentlyAdvertisedSession->bJoinable)
                    ? TEXT("true")
                    : TEXT("false"));
            ovr_GroupPresenceOptions_SetLobbySessionId(
                GroupPresenceOptions,
                TCHAR_TO_ANSI(*this->CurrentlyAdvertisedParty->RoomId->ToString()));
            ovr_GroupPresenceOptions_SetMatchSessionId(
                GroupPresenceOptions,
                TCHAR_TO_ANSI(*this->CurrentlyAdvertisedSession->RoomId->ToString()));
            ovr_GroupPresenceOptions_SetIsJoinable(
                GroupPresenceOptions,
                this->CurrentlyAdvertisedParty->bJoinable || this->CurrentlyAdvertisedSession->bJoinable);

            if (this->CurrentlyAdvertisedParty->bJoinable)
            {
                JsonObject->SetStringField(TEXT("PartyRoomId"), this->CurrentlyAdvertisedParty->RoomId->ToString());
                JsonObject->SetStringField(
                    TEXT("PartyRoomNamespace"),
                    this->CurrentlyAdvertisedParty->RoomNamespace.ToString());
            }
            if (this->CurrentlyAdvertisedSession->bJoinable)
            {
                JsonObject->SetStringField(TEXT("SessionRoomId"), this->CurrentlyAdvertisedSession->RoomId->ToString());
                JsonObject->SetStringField(
                    TEXT("SessionRoomNamespace"),
                    this->CurrentlyAdvertisedSession->RoomNamespace.ToString());
            }
        }
        // Advertise session only.
        else
        {
            UE_LOG(
                LogRedpointEOSPlatformIntegrationMeta,
                Verbose,
                TEXT("Advertising only the session '%s' on Meta Quest with bJoinable '%s'."),
                *this->CurrentlyAdvertisedSession->RoomId->ToString(),
                this->CurrentlyAdvertisedSession->bJoinable ? TEXT("true") : TEXT("false"));
            ovr_GroupPresenceOptions_SetLobbySessionId(
                GroupPresenceOptions,
                TCHAR_TO_ANSI(*this->CurrentlyAdvertisedSession->RoomId->ToString()));
            ovr_GroupPresenceOptions_SetIsJoinable(GroupPresenceOptions, this->CurrentlyAdvertisedSession->bJoinable);

            if (this->CurrentlyAdvertisedSession->bJoinable)
            {
                JsonObject->SetStringField(TEXT("SessionRoomId"), this->CurrentlyAdvertisedSession->RoomId->ToString());
                JsonObject->SetStringField(
                    TEXT("SessionRoomNamespace"),
                    this->CurrentlyAdvertisedSession->RoomNamespace.ToString());
            }
        }
    }

    // Serialize the JSON object and set it as our deeplink override.
    FString Deeplink;
    TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
        TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Deeplink);
    FJsonSerializer::Serialize(JsonObject, Writer);
    ovr_GroupPresenceOptions_SetDeeplinkMessageOverride(GroupPresenceOptions, TCHAR_TO_ANSI(*Deeplink));

    // Now set our group presence.
    UE_LOG(LogRedpointEOSPlatformIntegrationMeta, Verbose, TEXT("Setting group presence to deeplink: %s"), *Deeplink);

    // @note: This call frees the options once it's done with it.
    FMetaSDK::SetGroupPresence(
        GroupPresenceOptions,
        FMetaSDK::FOnSetGroupPresenceComplete::CreateSPLambda(
            this,
            [InCompletionDelegate](const FError &ErrorCode) {
                InCompletionDelegate.ExecuteIfBound(ErrorCode);
            }));
}

#endif

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteReceived &FMetaRuntimePlatformPresenceService::
    OnRoomAdvertisementInviteReceived(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    // @note: This event is never fired for Meta Quest, because invites are either immediately for accepting on
    // application launch, or the user has already accepted the invite through the Meta Quest UI.
    return this->OnRoomAdvertisementInviteReceivedDelegate;
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteAccepted &FMetaRuntimePlatformPresenceService::
    OnRoomAdvertisementInviteAccepted(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->OnRoomAdvertisementInviteAcceptedDelegate;
}

TOptional<EPresenceUserActivityState> FMetaRuntimePlatformPresenceService::GetNonUnifiedFriendPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId)
{
    if (FriendAccountType != EOS_EExternalAccountType::EOS_EAT_OCULUS)
    {
        return TOptional<EPresenceUserActivityState>();
    }

    auto FriendId = FMetaSDK::UserIdFromString(FriendAccountId);
    if (!FriendId.IsSet())
    {
        return TOptional<EPresenceUserActivityState>();
    }

    auto PresenceActivityState = this->FriendPresenceState.Find(FriendId.GetValue());
    if (PresenceActivityState == nullptr)
    {
        return TOptional<EPresenceUserActivityState>();
    }

    return *PresenceActivityState;
}

IRuntimePlatformPresenceService::FOnNonUnifiedFriendPresenceChanged &FMetaRuntimePlatformPresenceService::
    OnNonUnifiedFriendPresenceChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->OnNonUnifiedFriendPresenceChangedDelegate;
}

void FMetaRuntimePlatformPresenceService::SendRoomInviteToNonUnifiedFriend(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId,
    const FRoomIdRef &RoomId,
    const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate)
{
#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SEND_INVITES
    bool bCanSend = false;
    if (this->CurrentlyAdvertisedParty.IsSet() && this->CurrentlyAdvertisedParty->bJoinable &&
        this->CurrentlyAdvertisedParty->RoomId->IsEqual(RoomId))
    {
        bCanSend = true;
    }
    if (this->CurrentlyAdvertisedSession.IsSet() && this->CurrentlyAdvertisedSession->bJoinable &&
        this->CurrentlyAdvertisedSession->RoomId->IsEqual(RoomId))
    {
        bCanSend = true;
    }

    if (!bCanSend)
    {
        InCompletionDelegate.ExecuteIfBound(Errors::InvalidRequest(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The target room ID for sending the invite is not a room that the user is currently advertising as "
                 "a session or party, therefore a room invite can not be sent over the local platform.")));
        return;
    }

    if (FriendAccountType != EOS_EExternalAccountType::EOS_EAT_OCULUS)
    {
        InCompletionDelegate.ExecuteIfBound(Errors::InvalidRequest(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The target user for sending the invite is not a friend of the user on the local platform, therefore "
                 "a room invite can not be sent over the local platform.")));
        return;
    }

    if (!FMetaSDK::InitIfNeeded())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The Meta Quest SDK is not initialized, so invites can not be sent.")));
        return;
    }

    auto FriendAccountOvrId = FMetaSDK::UserIdFromString(FriendAccountId);
    if (!FriendAccountOvrId.IsSet())
    {
        InCompletionDelegate.ExecuteIfBound(Errors::InvalidRequest(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The target user's friend ID is not a valid Meta Quest account ID, therefore a room invite can not be "
                 "sent over the local platform.")));
        return;
    }

    UE_LOG(
        LogRedpointEOSPlatformIntegrationMeta,
        Verbose,
        TEXT("Sending a group presence invite to Meta Quest account '%s'."),
        *FriendAccountId);

    FMetaSDK::SendGroupPresenceInvite(
        FriendAccountOvrId.GetValue(),
        FMetaSDK::FOnSendGroupPresenceInviteComplete::CreateLambda([InCompletionDelegate](const FError &ErrorCode) {
            InCompletionDelegate.ExecuteIfBound(ErrorCode);
        }));
#else
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("This game is built against a version of the Meta Quest SDK that is too old to support presence "
             "operations. Please upgrade the Oculus Platform SDK or Unreal Engine to at least 5.5 for presence to "
             "work.")));
    return;
#endif
}

bool FMetaRuntimePlatformPresenceService::TryOpenRoomInviteRecipientSelection(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &RoomId)
{
#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_LAUNCH_INVITE_PANEL
    bool bCanSend = false;
    if (this->CurrentlyAdvertisedParty.IsSet() && this->CurrentlyAdvertisedParty->bJoinable &&
        this->CurrentlyAdvertisedParty->RoomId->IsEqual(RoomId))
    {
        bCanSend = true;
    }
    if (this->CurrentlyAdvertisedSession.IsSet() && this->CurrentlyAdvertisedSession->bJoinable &&
        this->CurrentlyAdvertisedSession->RoomId->IsEqual(RoomId))
    {
        bCanSend = true;
    }

    if (!bCanSend)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Error,
            TEXT("The target room ID for sending the invite is not a room that the user is currently advertising as "
                 "a session or party, therefore a room invite can not be sent over the local platform."));
        return false;
    }

    if (!FMetaSDK::InitIfNeeded())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Error,
            TEXT("The Meta Quest SDK is not initialized, so invites can not be sent."));
        return false;
    }

    UE_LOG(
        LogRedpointEOSPlatformIntegrationMeta,
        Verbose,
        TEXT("Opening the group presence invite panel for the user to invite friends."));

    FMetaSDK::LaunchGroupPresenceInvitePanel(
        FMetaSDK::FOnSendGroupPresenceInviteComplete::CreateLambda([](const FError &ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationMeta,
                    Error,
                    TEXT("Unable to open the Meta Quest invite panel: %s"),
                    *ErrorCode.ToLogString());
            }
        }));
    return true;
#else
    UE_LOG(
        LogRedpointEOSPlatformIntegrationMeta,
        Error,
        TEXT("This game is built against a version of the Meta Quest SDK that is too old to support presence "
             "operations. Please upgrade the Oculus Platform SDK or Unreal Engine to at least 5.5 for presence to "
             "work."));
    return false;
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()