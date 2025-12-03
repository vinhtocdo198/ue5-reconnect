// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointBase.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerDevice.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerDeviceList.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUserJoinedChannel.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUserPostLoginComplete.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUserRemoteUser.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUserSyncMode.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "VoiceChat.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    3738789065,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManager)

namespace REDPOINT_EOS_FILE_NS_ID(2877257062, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat;

class ONLINESUBSYSTEMREDPOINTEOS_API FEOSVoiceManagerLocalUser : public TSharedFromThis<FEOSVoiceManagerLocalUser>,
                                                                 public IHasEventRegistration
{
    friend FEOSVoiceManager;
    friend FEOSVoiceManagerLocalUserJoinedChannel;

private:
    FPlatformHandle PlatformHandle;
    TSharedRef<FEOSVoiceManager> VoiceManager;
    TSharedRef<const FUniqueNetIdEOS> LocalUserId;
    TMap<FString, FString> Settings;
    bool bHasSetCurrentInputDevice;
    FEOSVoiceManagerDevice CurrentInputDevice;
    bool bHasSetCurrentOutputDevice;
    FEOSVoiceManagerDevice CurrentOutputDevice;
    FDelegateHandle OnDevicesChangedHandle;
    bool bTransmitToAllChannels;
    TSet<FString> TransmitChannelNames;
    // Volumes are values between 0.0f and 200.0f.
    float InputVolume;
    float OutputVolume;
    bool bInputMuted;
    bool bOutputMuted;

    bool bHasSynchronisedThisFrame;
    bool bSynchroniseOnNextFrame;
    bool bIsSynchronising;
    EEOSVoiceManagerLocalUserSyncMode PendingSynchronisations;
    bool GetDefaultedSetting(const FString &InSettingName, bool InDefault);
    void PerformSynchronisationToEOS();

    TMap<FString, TSharedPtr<FEOSVoiceManagerLocalUserJoinedChannel>> JoinedChannels;

    void SetPlayersBlockState(const TArray<FString> &PlayerNames, bool bBlocked);

    FOnVoiceChatAvailableAudioDevicesChangedDelegate OnVoiceChatAvailableAudioDevicesChangedDelegate;
    FOnVoiceChatChannelJoinedDelegate OnVoiceChatChannelJoinedDelegate;
    FOnVoiceChatChannelExitedDelegate OnVoiceChatChannelExitedDelegate;
    FOnVoiceChatCallStatsUpdatedDelegate OnVoiceChatCallStatsUpdatedDelegate;
    FOnVoiceChatPlayerAddedDelegate OnVoiceChatPlayerAddedDelegate;
    FOnVoiceChatPlayerRemovedDelegate OnVoiceChatPlayerRemovedDelegate;
    FOnVoiceChatPlayerTalkingUpdatedDelegate OnVoiceChatPlayerTalkingUpdatedDelegate;
    FOnVoiceChatPlayerMuteUpdatedDelegate OnVoiceChatPlayerMuteUpdatedDelegate;
    FOnVoiceChatPlayerVolumeUpdatedDelegate OnVoiceChatPlayerVolumeUpdatedDelegate;

    void SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode SyncMode = EEOSVoiceManagerLocalUserSyncMode::All);

    void RegisterLobbyChannel(
        const TSharedRef<const FOnlinePartyIdRedpointBase> &InLobbyId,
        const FString &ChannelName,
        EVoiceChatChannelType ChannelType);
    void UnregisterLobbyChannel(const TSharedRef<const FOnlinePartyIdRedpointBase> &InLobbyId);

    void OnAvailableAudioDevicesChanged();

public:
    FEOSVoiceManagerLocalUser(
        FPlatformHandle InPlatformHandle,
        const TSharedRef<FEOSVoiceManager> &InVoiceManager,
        const TSharedRef<const FUniqueNetIdEOS> &InLocalUserId);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FEOSVoiceManagerLocalUser);
    virtual ~FEOSVoiceManagerLocalUser() override;

    void PostLoginInit(const FEOSVoiceManagerLocalUserPostLoginComplete &OnPostLoginComplete);

    void SetSetting(const FString &Name, const FString &Value);
    FString GetSetting(const FString &Name);
    void SetAudioInputVolume(float Volume);
    void SetAudioOutputVolume(float Volume);
    float GetAudioInputVolume() const;
    float GetAudioOutputVolume() const;
    void SetAudioInputDeviceMuted(bool bIsMuted);
    void SetAudioOutputDeviceMuted(bool bIsMuted);
    bool GetAudioInputDeviceMuted() const;
    bool GetAudioOutputDeviceMuted() const;
    TArray<FEOSVoiceManagerDevice> GetAvailableInputDeviceInfos() const;
    TArray<FEOSVoiceManagerDevice> GetAvailableOutputDeviceInfos() const;
    void SetInputDeviceId(const FString &InputDeviceId);
    void SetOutputDeviceId(const FString &OutputDeviceId);
    FEOSVoiceManagerDevice GetInputDeviceInfo() const;
    FEOSVoiceManagerDevice GetOutputDeviceInfo() const;
    FEOSVoiceManagerDevice GetDefaultInputDeviceInfo() const;
    FEOSVoiceManagerDevice GetDefaultOutputDeviceInfo() const;
    void BlockPlayers(const TArray<FString> &PlayerNames);
    void UnblockPlayers(const TArray<FString> &PlayerNames);
    void JoinChannel(
        const FString &ChannelName,
        const FString &ChannelCredentials,
        EVoiceChatChannelType ChannelType,
        const FOnVoiceChatChannelJoinCompleteDelegate &Delegate,
        const TOptional<FVoiceChatChannel3dProperties> &Channel3dProperties);
    void LeaveChannel(const FString &ChannelName, const FOnVoiceChatChannelLeaveCompleteDelegate &Delegate);
    void Set3DPosition(
        const FString &ChannelName,
        const FVector &SpeakerPosition,
        const FVector &ListenerPosition,
        const FVector &ListenerForwardDirection,
        const FVector &ListenerUpDirection);
    TArray<FString> GetChannels();
    const TArray<FString> GetPlayersInChannel(const FString &ChannelName) const;
    EVoiceChatChannelType GetChannelType(const FString &ChannelName) const;
    bool IsPlayerTalking(const FString &PlayerName) const;
    void SetPlayerMuted(const FString &PlayerName, bool bMuted);
    bool IsPlayerMuted(const FString &PlayerName) const;
    void SetPlayerVolume(const FString &PlayerName, float Volume);
    float GetPlayerVolume(const FString &PlayerName) const;
    void TransmitToAllChannels();
    void TransmitToNoChannels();
#if REDPOINT_EOS_UE_5_3_OR_LATER
    void TransmitToSpecificChannels(const TSet<FString> &ChannelNames);
#else
    void TransmitToSpecificChannel(const FString &ChannelName);
#endif
    EVoiceChatTransmitMode GetTransmitMode() const;
#if REDPOINT_EOS_UE_5_3_OR_LATER
    TSet<FString> GetTransmitChannels() const;
#else
    FString GetTransmitChannel() const;
#endif

    FOnVoiceChatAvailableAudioDevicesChangedDelegate &OnVoiceChatAvailableAudioDevicesChanged();
    FOnVoiceChatChannelJoinedDelegate &OnVoiceChatChannelJoined();
    FOnVoiceChatChannelExitedDelegate &OnVoiceChatChannelExited();
    FOnVoiceChatCallStatsUpdatedDelegate &OnVoiceChatCallStatsUpdated();
    FOnVoiceChatPlayerAddedDelegate &OnVoiceChatPlayerAdded();
    FOnVoiceChatPlayerRemovedDelegate &OnVoiceChatPlayerRemoved();
    FOnVoiceChatPlayerTalkingUpdatedDelegate &OnVoiceChatPlayerTalkingUpdated();
    FOnVoiceChatPlayerMuteUpdatedDelegate &OnVoiceChatPlayerMuteUpdated();
    FOnVoiceChatPlayerVolumeUpdatedDelegate &OnVoiceChatPlayerVolumeUpdated();
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(2877257062, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat, FEOSVoiceManagerLocalUser)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()