// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/VoiceChatStats.h"
#include "RedpointEOSAPI/Lobby/GetRTCRoomName.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3738789065, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{
using namespace ::Redpoint::EOS::API::Lobby;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

void FEOSVoiceManager::Internal_UserJoinedLobby(
    const TSharedRef<const FUniqueNetIdEOS> &UserId,
    const TSharedRef<const FOnlinePartyIdRedpointBase> &LobbyId)
{
    // Get the RTC room name for this lobby.
    EOS_EResult GetRoomNameResult;
    FString RoomName;
    FGetRTCRoomName::Execute(
        this->PlatformHandle,
        FGetRTCRoomName::Options{UserId->GetProductUserId(), LobbyId->GetRoomId()->GetRoomId()},
        GetRoomNameResult,
        RoomName);
    if (GetRoomNameResult != EOS_EResult::EOS_Success)
    {
        if (GetRoomNameResult == EOS_EResult::EOS_Disabled)
        {
            // This is expected if the lobby does not have RTC enabled.
        }
        else
        {
            UE_LOG(
                LogRedpointEOSVoiceChat,
                Error,
                TEXT("%s"),
                *Redpoint::EOS::API::ConvertError(
                     TEXT("EOS_Lobby_GetRTCRoomName"),
                     TEXT("Unable to retrieve room name for RTC-enabled lobby"),
                     GetRoomNameResult)
                     .ToLogString());
        }
        return;
    }

    // Check if echo is enabled.
    bool IsEchoEnabled = false;
    for (int i = QueuedEchoEnabledLobbies.Num() - 1; i >= 0; i--)
    {
        const auto &Entry = QueuedEchoEnabledLobbies[i];
        if (*Entry.Key == *UserId && *Entry.Value == *LobbyId)
        {
            IsEchoEnabled = true;
            QueuedEchoEnabledLobbies.RemoveAt(i);
            break;
        }
    }

    // Register the lobby channel with RTC.
    auto LocalUser = this->GetLocalUser(*UserId);
    if (!LocalUser.IsValid())
    {
        UE_LOG(
            LogRedpointEOSVoiceChat,
            Fatal,
            TEXT("Local voice chat user has not been added at the point of the user joining an RTC enabled room. We "
                 "are unable to register the lobby channel with the voice chat user. This typically indicates you are "
                 "trying to create or join a lobby prior to FVoiceManagerIdentityHook::OnStartSystem running, or after "
                 "FVoiceManagerIdentityHook::OnStopSystem has run."));
    }
    LocalUser->RegisterLobbyChannel(
        LobbyId,
        RoomName,
        IsEchoEnabled ? EVoiceChatChannelType::Echo : EVoiceChatChannelType::NonPositional);
}

void FEOSVoiceManager::Internal_UserLeftLobby(
    const TSharedRef<const FUniqueNetIdEOS> &UserId,
    const TSharedRef<const FOnlinePartyIdRedpointBase> &LobbyId)
{
    auto LocalUser = this->GetLocalUser(*UserId);
    if (LocalUser.IsValid())
    {
        LocalUser->UnregisterLobbyChannel(LobbyId);
    }
}

void FEOSVoiceManager::OnLobbyMemberStatusReceived(const FNotifyLobbyMemberStatusReceived::Result &Result)
{
    auto UserId = MakeShared<FUniqueNetIdEOS>(Result.TargetUserId);
    auto LobbyId = FOnlinePartyIdRedpointBase::Create(
        MakeShared<Redpoint::EOS::Rooms::FRoomId>(FName(TEXT("Lobby")), Result.LobbyId));

    // We're only interested in when local users join or leave lobbies.
    if (this->Identity->GetLoginStatus(*UserId) != ELoginStatus::LoggedIn)
    {
        return;
    }

    // Figure out what to do based on the status change.
    switch (Result.CurrentStatus)
    {
    case EOS_ELobbyMemberStatus::EOS_LMS_JOINED:
        // This fires when *other* users join the lobby, which we don't need to track.
    case EOS_ELobbyMemberStatus::EOS_LMS_LEFT:
        // This fires when *other* users leave the lobby, which we don't need to track.
    case EOS_ELobbyMemberStatus::EOS_LMS_PROMOTED:
        // Irrelevant for voice chat.
        break;
    case EOS_ELobbyMemberStatus::EOS_LMS_DISCONNECTED:
        // This can fire when *we* leave the lobby, so we need to handle this.
        UE_LOG(
            LogRedpointEOSVoiceChat,
            Verbose,
            TEXT("lobby %s, user %s: LobbyMemberStatusReceived with EOS_LMS_DISCONNECTED."),
            *LobbyId->ToString(),
            *UserId->ToString());
        this->Internal_UserLeftLobby(UserId, LobbyId);
        break;
    case EOS_ELobbyMemberStatus::EOS_LMS_KICKED:
        // This can fire when *we* are kicked from the lobby, so we need to handle this.
        UE_LOG(
            LogRedpointEOSVoiceChat,
            Verbose,
            TEXT("lobby %s, user %s: LobbyMemberStatusReceived with EOS_LMS_KICKED."),
            *LobbyId->ToString(),
            *UserId->ToString());
        this->Internal_UserLeftLobby(UserId, LobbyId);
        break;
    case EOS_ELobbyMemberStatus::EOS_LMS_CLOSED:
        // This can fire when *we* are removed from the lobby, because the lobby was deleted, so we need to handle this.
        UE_LOG(
            LogRedpointEOSVoiceChat,
            Verbose,
            TEXT("lobby %s, user %s: LobbyMemberStatusReceived with EOS_LMS_CLOSED."),
            *LobbyId->ToString(),
            *UserId->ToString());
        this->Internal_UserLeftLobby(UserId, LobbyId);
        break;
    default:
        checkf(false, TEXT("Unsupported EOS_ELobbyMemberStatus value in FEOSVoiceManager."));
        break;
    }
}

FEOSVoiceManager::FEOSVoiceManager(
    FPlatformHandle InPlatformHandle,
    const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig,
    const TSharedRef<IOnlineIdentity, ESPMode::ThreadSafe> &InIdentity)
    : PlatformHandle(InPlatformHandle)
    , Config(InConfig)
    , Identity(InIdentity)
    , LocalUsers()
    , QueuedEchoEnabledLobbies()
    , DeviceList(MakeSharedWithEvents<FEOSVoiceManagerDeviceList>(InPlatformHandle))
{
}

void FEOSVoiceManager::RegisterEvents()
{
    this->OnNativeLobbyMemberStatusReceived = FNotifyLobbyMemberStatusReceived::Register(
        this->PlatformHandle,
        FNotifyLobbyMemberStatusReceived::Options{},
        FNotifyLobbyMemberStatusReceived::EventDelegate::CreateSP(
            this,
            &FEOSVoiceManager::OnLobbyMemberStatusReceived));
}

void FEOSVoiceManager::MarkPartyOrLobbyAsEchoEnabled(
    const TSharedRef<const FUniqueNetIdEOS> &InUserId,
    const TSharedRef<const FOnlinePartyId> &InPartyOrLobbyId)
{
    // Check to see if the lobby is already known to this user.
    auto User = this->GetLocalUser(*InUserId);
    if (!User.IsValid())
    {
        // Queue it for later.
        this->QueuedEchoEnabledLobbies.Add(
            TPair<TSharedRef<const FUniqueNetIdEOS>, TSharedRef<const FOnlinePartyId>>(InUserId, InPartyOrLobbyId));
        return;
    }
    for (const auto &Channel : User->JoinedChannels)
    {
        if (Channel.Value->LobbyId.IsValid() && *Channel.Value->LobbyId == *InPartyOrLobbyId)
        {
            // Found the existing channel, just change it's mode.
            Channel.Value->ChannelType = EVoiceChatChannelType::Echo;
            return;
        }
    }

    // Queue it for later.
    this->QueuedEchoEnabledLobbies.Add(
        TPair<TSharedRef<const FUniqueNetIdEOS>, TSharedRef<const FOnlinePartyId>>(InUserId, InPartyOrLobbyId));
    return;
}

void FEOSVoiceManager::ScheduleUserSynchronisationIfNeeded()
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChat, SyncPerformDeferredSynchronisation);

    for (const auto &LocalUser : this->LocalUsers)
    {
        if (LocalUser.Value->bSynchroniseOnNextFrame)
        {
            LocalUser.Value->bHasSynchronisedThisFrame = false;
            LocalUser.Value->bSynchroniseOnNextFrame = false;
            LocalUser.Value->PerformSynchronisationToEOS();
        }
    }
}

void FEOSVoiceManager::UserManuallyJoinedLobby(const FUniqueNetIdEOS &UserId, const FOnlinePartyIdRedpointBase &LobbyId)
{
    UE_LOG(
        LogRedpointEOSVoiceChat,
        Verbose,
        TEXT("lobby %s, user %s: UserManuallyJoinedLobby"),
        *LobbyId.ToString(),
        *UserId.ToString());

    this->Internal_UserJoinedLobby(
        StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()),
        StaticCastSharedRef<const FOnlinePartyIdRedpointBase>(LobbyId.AsShared()));
}

void FEOSVoiceManager::UserManuallyLeftLobby(const FUniqueNetIdEOS &UserId, const FOnlinePartyIdRedpointBase &LobbyId)
{
    UE_LOG(
        LogRedpointEOSVoiceChat,
        Verbose,
        TEXT("lobby %s, user %s: UserManuallyLeftLobby"),
        *LobbyId.ToString(),
        *UserId.ToString());

    this->Internal_UserLeftLobby(
        StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()),
        StaticCastSharedRef<const FOnlinePartyIdRedpointBase>(LobbyId.AsShared()));
}

void FEOSVoiceManager::AddLocalUser(const FUniqueNetIdEOS &UserId)
{
    this->LocalUsers.Add(
        UserId,
        MakeSharedWithEvents<FEOSVoiceManagerLocalUser>(
            this->PlatformHandle,
            this->AsShared(),
            StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared())));
}

void FEOSVoiceManager::RemoveLocalUser(const FUniqueNetIdEOS &UserId)
{
    if (this->LocalUsers.Contains(UserId))
    {
        // Channels have references to their owning users, so we have to clear out the joined channels so the users
        // don't stay alive due to circular references.
        this->LocalUsers[UserId]->JoinedChannels.Empty();
    }
    this->LocalUsers.Remove(UserId);
}

void FEOSVoiceManager::RemoveAllLocalUsers()
{
    for (const auto &KV : this->LocalUsers)
    {
        // Channels have references to their owning users, so we have to clear out the joined channels so the users
        // don't stay alive due to circular references.
        KV.Value->JoinedChannels.Empty();
    }
    this->LocalUsers.Empty();
}

TSharedPtr<FEOSVoiceManagerLocalUser> FEOSVoiceManager::GetLocalUser(const FUniqueNetIdEOS &UserId)
{
    if (this->LocalUsers.Contains(UserId))
    {
        return this->LocalUsers[UserId];
    }
    return nullptr;
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()