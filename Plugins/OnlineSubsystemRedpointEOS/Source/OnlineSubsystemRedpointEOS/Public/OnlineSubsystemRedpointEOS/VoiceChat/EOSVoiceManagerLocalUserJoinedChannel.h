// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointBase.h"
#include "RedpointEOSAPI/RTC/NotifyDisconnected.h"
#include "RedpointEOSAPI/RTC/NotifyParticipantStatusChanged.h"
#include "RedpointEOSAPI/RTCAudio/NotifyAudioInputState.h"
#include "RedpointEOSAPI/RTCAudio/NotifyAudioOutputState.h"
#include "RedpointEOSAPI/RTCAudio/NotifyParticipantUpdated.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "VoiceChat.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    2877257062,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerLocalUser)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    3314572764,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerLocalUserRemoteUser)

namespace REDPOINT_EOS_FILE_NS_ID(1057666991, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::RTC;
using namespace ::Redpoint::EOS::API::RTCAudio;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat;

class ONLINESUBSYSTEMREDPOINTEOS_API FEOSVoiceManagerLocalUserJoinedChannel
    : public TSharedFromThis<FEOSVoiceManagerLocalUserJoinedChannel>,
      public IHasEventRegistration
{
public:
    TSharedRef<FEOSVoiceManagerLocalUser> Owner;
    FPlatformHandle PlatformHandle;
    FString RoomName;
    TSharedPtr<const FOnlinePartyIdRedpointBase> LobbyId;
    EVoiceChatChannelType ChannelType;
    TUserIdMap<TSharedPtr<FEOSVoiceManagerLocalUserRemoteUser>> Participants;
    FEventHandlePtr OnNativeDisconnected;
    FEventHandlePtr OnNativeParticipantStatusChanged;
    FEventHandlePtr OnNativeParticipantUpdated;
    // FEventHandlePtr OnNativeAudioInputState;
    // FEventHandlePtr OnNativeAudioOutputState;
    // EOS_RTCAudio_AddNotifyAudioBeforeSend
    // EOS_RTCAudio_AddNotifyAudioBeforeRender

private:
    void OnDisconnected(const FNotifyDisconnected::Result &Result);
    void OnParticipantStatusChanged(const FNotifyParticipantStatusChanged::Result &Result);
    void OnParticipantUpdated(const FNotifyParticipantUpdated::Result &Result);

public:
    FEOSVoiceManagerLocalUserJoinedChannel(
        const TSharedRef<FEOSVoiceManagerLocalUser> &InOwner,
        FPlatformHandle InPlatformHandle,
        const FString &InRoomName,
        EVoiceChatChannelType InChannelType);
    FEOSVoiceManagerLocalUserJoinedChannel(
        const TSharedRef<FEOSVoiceManagerLocalUser> &InOwner,
        FPlatformHandle InPlatformHandle,
        const FString &InRoomName,
        EVoiceChatChannelType InChannelType,
        const TSharedRef<const FOnlinePartyIdRedpointBase> &InLobbyId);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FEOSVoiceManagerLocalUserJoinedChannel);
    virtual ~FEOSVoiceManagerLocalUserJoinedChannel() override = default;

    static TSharedRef<FEOSVoiceManagerLocalUserJoinedChannel> NewDedicated(
        const TSharedRef<FEOSVoiceManagerLocalUser> &InOwner,
        FPlatformHandle InPlatformHandle,
        const FString &InRoomName,
        EVoiceChatChannelType InChannelType);
    static TSharedRef<FEOSVoiceManagerLocalUserJoinedChannel> NewLobby(
        const TSharedRef<FEOSVoiceManagerLocalUser> &InOwner,
        FPlatformHandle InPlatformHandle,
        const FString &InRoomName,
        const TSharedRef<const FOnlinePartyIdRedpointBase> &InLobbyId,
        EVoiceChatChannelType InChannelType);
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1057666991,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerLocalUserJoinedChannel)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()