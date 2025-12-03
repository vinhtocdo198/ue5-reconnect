// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUserJoinedChannel.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUser.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUserRemoteUser.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1057666991, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

void FEOSVoiceManagerLocalUserJoinedChannel::OnDisconnected(const FNotifyDisconnected::Result &Result)
{
    if (Result.ResultCode == EOS_EResult::EOS_Success)
    {
        // Room left because of a call to LeaveRoom or because an RTC-enabled lobby was left.
        this->Owner->OnVoiceChatChannelExited().Broadcast(this->RoomName, FVoiceChatResult(EVoiceChatResult::Success));
        this->Owner->JoinedChannels.Remove(this->RoomName);
    }
    else if (Result.ResultCode == EOS_EResult::EOS_NoConnection)
    // NOLINTNEXTLINE(bugprone-branch-clone)
    {
        // Temporary interruption.
        // @todo: Will EOS automatically reconnect here?
    }
    else if (Result.ResultCode == EOS_EResult::EOS_RTC_UserKicked)
    {
        // User was kicked from the RTC lobby.
        this->Owner->OnVoiceChatChannelExited().Broadcast(
            this->RoomName,
            FVoiceChatResult(
                EVoiceChatResult::NotPermitted,
                TEXT("KickedByServer"),
                TEXT("You were kicked from voice chat by the server.")));
        this->Owner->JoinedChannels.Remove(this->RoomName);
    }
    else if (Result.ResultCode == EOS_EResult::EOS_ServiceFailure)
    {
        // Temporary interruption.
        // @todo: Will EOS automatically reconnect here?
    }
    else
    {
        // Unknown.
        // @todo: Are we actually disconnected from the room here?
    }
}

void FEOSVoiceManagerLocalUserJoinedChannel::OnParticipantStatusChanged(
    const FNotifyParticipantStatusChanged::Result &Result)
{
    if (Result.ParticipantStatus == EOS_ERTCParticipantStatus::EOS_RTCPS_Joined)
    {
        TSharedRef<const FUniqueNetIdEOS> RemoteUserId = MakeShared<FUniqueNetIdEOS>(Result.ParticipantId);
        // @todo: Grab metadata and find some way of surfacing it...
        this->Participants.Add(*RemoteUserId, MakeShared<FEOSVoiceManagerLocalUserRemoteUser>(RemoteUserId));
        this->Owner->OnVoiceChatPlayerAdded().Broadcast(this->RoomName, RemoteUserId->ToString());
    }
    else if (Result.ParticipantStatus == EOS_ERTCParticipantStatus::EOS_RTCPS_Left)
    {
        TSharedRef<const FUniqueNetIdEOS> RemoteUserId = MakeShared<FUniqueNetIdEOS>(Result.ParticipantId);
        this->Participants.Remove(*RemoteUserId);
        this->Owner->OnVoiceChatPlayerRemoved().Broadcast(this->RoomName, RemoteUserId->ToString());
    }
}

void FEOSVoiceManagerLocalUserJoinedChannel::OnParticipantUpdated(const FNotifyParticipantUpdated::Result &Result)
{
    TSharedRef<const FUniqueNetIdEOS> RemoteUserId = MakeShared<FUniqueNetIdEOS>(Result.ParticipantId);
    if (this->Participants.Contains(*RemoteUserId))
    {
        auto &RemoteUser = this->Participants[*RemoteUserId];
        bool bSpeaking = Result.bSpeaking == EOS_TRUE;
        if (RemoteUser->bIsTalking != bSpeaking)
        {
            RemoteUser->bIsTalking = bSpeaking;
            this->Owner->OnVoiceChatPlayerTalkingUpdated().Broadcast(
                this->RoomName,
                RemoteUserId->ToString(),
                RemoteUser->bIsTalking);
        }
        if (Redpoint::EOS::Config::ApiVersionIsAtLeast(*this->Owner->VoiceManager->Config, EEOSApiVersion::v2023_10_27))
        {
            // As of 2023-10-27, we no longer sync bIsMuted based on the remote mute status.
        }
        else
        {
            bool bIsMuted = Result.AudioStatus != EOS_ERTCAudioStatus::EOS_RTCAS_Enabled;
            if (RemoteUser->bIsMuted != bIsMuted)
            {
                FString AudioStatusString = TEXT("Unknown");
                switch (Result.AudioStatus)
                {
                case EOS_ERTCAudioStatus::EOS_RTCAS_Enabled:
                    AudioStatusString = TEXT("Enabled");
                    break;
                case EOS_ERTCAudioStatus::EOS_RTCAS_Disabled:
                    AudioStatusString = TEXT("Disabled");
                    break;
                case EOS_ERTCAudioStatus::EOS_RTCAS_AdminDisabled:
                    AudioStatusString = TEXT("AdminDisabled");
                    break;
                case EOS_ERTCAudioStatus::EOS_RTCAS_NotListeningDisabled:
                    AudioStatusString = TEXT("NotListeningDisabled");
                    break;
                case EOS_ERTCAudioStatus::EOS_RTCAS_Unsupported:
                default:
                    AudioStatusString = TEXT("Unsupported");
                    break;
                }
                UE_LOG(
                    LogRedpointEOSVoiceChat,
                    Verbose,
                    TEXT("Voice chat participant status updated. Channel: '%s', Player: '%s', Audio Status: '%s'"),
                    *this->RoomName,
                    *RemoteUserId->ToString(),
                    *AudioStatusString);

                RemoteUser->bIsMuted = bIsMuted;
                this->Owner->OnVoiceChatPlayerMuteUpdated().Broadcast(
                    this->RoomName,
                    RemoteUserId->ToString(),
                    bIsMuted);
            }
        }
    }
}

FEOSVoiceManagerLocalUserJoinedChannel::FEOSVoiceManagerLocalUserJoinedChannel(
    const TSharedRef<FEOSVoiceManagerLocalUser> &InOwner,
    FPlatformHandle InPlatformHandle,
    const FString &InRoomName,
    EVoiceChatChannelType InChannelType)
    : Owner(InOwner)
    , PlatformHandle(InPlatformHandle)
    , RoomName(InRoomName)
    , LobbyId()
    , ChannelType(InChannelType)
    , Participants()
    , OnNativeDisconnected()
    , OnNativeParticipantStatusChanged()
    , OnNativeParticipantUpdated()
{
}

FEOSVoiceManagerLocalUserJoinedChannel::FEOSVoiceManagerLocalUserJoinedChannel(
    const TSharedRef<FEOSVoiceManagerLocalUser> &InOwner,
    FPlatformHandle InPlatformHandle,
    const FString &InRoomName,
    EVoiceChatChannelType InChannelType,
    const TSharedRef<const FOnlinePartyIdRedpointBase> &InLobbyId)
    : FEOSVoiceManagerLocalUserJoinedChannel(InOwner, InPlatformHandle, InRoomName, InChannelType)
{
    this->LobbyId = InLobbyId;
}

void FEOSVoiceManagerLocalUserJoinedChannel::RegisterEvents()
{
    this->OnNativeDisconnected = FNotifyDisconnected::Register(
        this->PlatformHandle,
        FNotifyDisconnected::Options{this->Owner->LocalUserId->GetProductUserId(), this->RoomName},
        FNotifyDisconnected::EventDelegate::CreateSP(this, &FEOSVoiceManagerLocalUserJoinedChannel::OnDisconnected));

    this->OnNativeParticipantStatusChanged = FNotifyParticipantStatusChanged::Register(
        this->PlatformHandle,
        FNotifyParticipantStatusChanged::Options{this->Owner->LocalUserId->GetProductUserId(), this->RoomName},
        FNotifyParticipantStatusChanged::EventDelegate::CreateSP(
            this,
            &FEOSVoiceManagerLocalUserJoinedChannel::OnParticipantStatusChanged));

    this->OnNativeParticipantUpdated = FNotifyParticipantUpdated::Register(
        this->PlatformHandle,
        FNotifyParticipantUpdated::Options{this->Owner->LocalUserId->GetProductUserId(), this->RoomName},
        FNotifyParticipantUpdated::EventDelegate::CreateSP(
            this,
            &FEOSVoiceManagerLocalUserJoinedChannel::OnParticipantUpdated));
}

TSharedRef<FEOSVoiceManagerLocalUserJoinedChannel> FEOSVoiceManagerLocalUserJoinedChannel::NewDedicated(
    const TSharedRef<FEOSVoiceManagerLocalUser> &InOwner,
    FPlatformHandle InPlatformHandle,
    const FString &InRoomName,
    EVoiceChatChannelType InChannelType)
{
    return MakeSharedWithEvents<FEOSVoiceManagerLocalUserJoinedChannel>(
        InOwner,
        InPlatformHandle,
        InRoomName,
        InChannelType);
}

TSharedRef<FEOSVoiceManagerLocalUserJoinedChannel> FEOSVoiceManagerLocalUserJoinedChannel::NewLobby(
    const TSharedRef<FEOSVoiceManagerLocalUser> &InOwner,
    FPlatformHandle InPlatformHandle,
    const FString &InRoomName,
    const TSharedRef<const FOnlinePartyIdRedpointBase> &InLobbyId,
    EVoiceChatChannelType InChannelType)
{
    return MakeSharedWithEvents<FEOSVoiceManagerLocalUserJoinedChannel>(
        InOwner,
        InPlatformHandle,
        InRoomName,
        InChannelType,
        InLobbyId);
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()