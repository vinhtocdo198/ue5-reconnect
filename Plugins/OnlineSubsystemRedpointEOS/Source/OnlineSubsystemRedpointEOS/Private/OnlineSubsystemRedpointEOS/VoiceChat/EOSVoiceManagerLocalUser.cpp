// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUser.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/SetAudioInputSettingsVoiceTask.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/SetAudioInputVolumeVoiceTask.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/SetAudioOutputSettingsVoiceTask.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/SetAudioOutputVolumeVoiceTask.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/Tasks/VoiceTask.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/VoiceChatStats.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/RTC/BlockParticipant.h"
#include "RedpointEOSAPI/RTC/JoinRoom.h"
#include "RedpointEOSAPI/RTC/LeaveRoom.h"
#include "RedpointEOSAPI/RTCAudio/UpdateReceiving.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2877257062, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::Tasks;

bool FEOSVoiceManagerLocalUser::GetDefaultedSetting(const FString &InSettingName, bool InDefault)
{
    if (this->Settings.Contains(InSettingName))
    {
        return this->Settings[InSettingName] == "true";
    }
    else
    {
        return InDefault;
    }
}

void FEOSVoiceManagerLocalUser::PerformSynchronisationToEOS()
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChatUser, SyncPerformSynchronisation);

    UE_LOG(LogRedpointEOSVoiceChat, Verbose, TEXT("Synchronising voice chat state with EOS..."));

    checkf(
        !this->bIsSynchronising,
        TEXT("PerformSynchronisationToEOS called while bIsSynchronising is true; this is a bug!"));

    this->bIsSynchronising = true;
    auto SyncMode = this->PendingSynchronisations;
    this->PendingSynchronisations = EEOSVoiceManagerLocalUserSyncMode::None;

    TArray<TSharedRef<FVoiceTask>> VoiceTasksToRun;

    if ((SyncMode & EEOSVoiceManagerLocalUserSyncMode::InputSettings) != 0)
    {
        FSetAudioInputSettingsVoiceTaskData Data;
        Data.PlatformHandle = this->PlatformHandle;
        Data.LocalUserId = this->LocalUserId->GetProductUserId();
        Data.bHasSetCurrentInputDevice = this->bHasSetCurrentInputDevice;
        Data.CurrentInputDeviceId = this->CurrentInputDevice.Id;
        Data.DefaultInputDeviceId = this->GetDefaultInputDeviceInfo().Id;
        Data.bUserInputMuted = this->bInputMuted || this->InputVolume == 0.0f;
        Data.UserInputVolume = this->InputVolume;
        Data.bEnableEchoCancellation = this->GetDefaultedSetting("EnableEchoCancellation", true);
        Data.bEnableNoiseSuppression = this->GetDefaultedSetting("EnableNoiseSuppression", true);
        Data.bEnableAutoGainControl = this->GetDefaultedSetting("EnableAutoGainControl", true);
        Data.bEnableDtx = this->GetDefaultedSetting("EnableDtx", true);
        Data.bEnablePlatformAEC = this->GetDefaultedSetting(
            // Use the legacy option first if the developer is providing it.
            "PlatformAECEnabled",
            this->GetDefaultedSetting(
                // Otherwise use the new option.
                "EnablePlatformAEC",
                // This default is effectively ignored because the constructor
                // sets the setting to whatever GetEnableVoiceChatPlatformAECByDefault
                // has in the config (from Project Settings).
                true));

        VoiceTasksToRun.Add(MakeShared<FSetAudioInputSettingsVoiceTask>(Data));
    }

    if ((SyncMode & EEOSVoiceManagerLocalUserSyncMode::TransmitMode) != 0 ||
        (SyncMode & EEOSVoiceManagerLocalUserSyncMode::InputSettings) != 0 ||
        (SyncMode & EEOSVoiceManagerLocalUserSyncMode::JoinedRoom) != 0)
    {
        FSetAudioInputVolumeVoiceTaskData Data;
        Data.PlatformHandle = this->PlatformHandle;
        Data.LocalUserId = this->LocalUserId->GetProductUserId();
        for (const auto &JoinedChannel : this->JoinedChannels)
        {
            Data.RoomNamesWithTransmitStatus.Add(FSetAudioInputVolumeVoiceTaskData::FRoomNameWithTransmitStatus{
                JoinedChannel.Key,
                this->bTransmitToAllChannels || this->TransmitChannelNames.Contains(JoinedChannel.Key)});
        }
        Data.bUserInputMuted = this->bInputMuted || this->InputVolume == 0.0f;
        Data.UserInputVolume = this->InputVolume;
        VoiceTasksToRun.Add(MakeShared<FSetAudioInputVolumeVoiceTask>(Data));
    }

    if ((SyncMode & EEOSVoiceManagerLocalUserSyncMode::OutputSettings) != 0)
    {
        FSetAudioOutputSettingsVoiceTaskData Data;
        Data.PlatformHandle = this->PlatformHandle;
        Data.LocalUserId = this->LocalUserId->GetProductUserId();
        Data.bHasSetCurrentOutputDevice = this->bHasSetCurrentOutputDevice;
        Data.CurrentOutputDeviceId = this->CurrentOutputDevice.Id;
        Data.DefaultOutputDeviceId = this->GetDefaultOutputDeviceInfo().Id;
        Data.bUserOutputMuted = this->bOutputMuted || this->OutputVolume == 0.0f;
        Data.UserOutputVolume = this->OutputVolume;

        VoiceTasksToRun.Add(MakeShared<FSetAudioOutputSettingsVoiceTask>(Data));
    }

    if ((SyncMode & EEOSVoiceManagerLocalUserSyncMode::OutputSettings) != 0 ||
        (SyncMode & EEOSVoiceManagerLocalUserSyncMode::JoinedRoom) != 0)
    {
        FSetAudioOutputVolumeVoiceTaskData Data;
        Data.PlatformHandle = this->PlatformHandle;
        Data.LocalUserId = this->LocalUserId->GetProductUserId();
        this->JoinedChannels.GetKeys(Data.RoomNames);
        Data.bUserOutputMuted = this->bOutputMuted || this->OutputVolume == 0.0f;
        Data.UserOutputVolume = this->OutputVolume;

        VoiceTasksToRun.Add(MakeShared<FSetAudioOutputVolumeVoiceTask>(Data));
    }

    // If there's nothing computed to run, immediately finish.
    if (VoiceTasksToRun.Num() == 0)
    {
        UE_LOG(
            LogRedpointEOSVoiceChat,
            Warning,
            TEXT("Finished synchronising voice chat state with EOS with nothing to action."));

        this->bIsSynchronising = false;

        // Kick off another sync operation if we need to.
        if (this->PendingSynchronisations != EEOSVoiceManagerLocalUserSyncMode::None)
        {
            this->PerformSynchronisationToEOS();
        }

        return;
    }

    FMultiOperation<TSharedRef<FVoiceTask>, bool>::RunSP(
        this,
        VoiceTasksToRun,
        [](const TSharedRef<FVoiceTask> &InVoiceTask, const TFunction<void(bool OutValue)> &OnDone) -> bool {
            UE_LOG(
                LogRedpointEOSVoiceChat,
                Verbose,
                TEXT("Synchronising voice chat with task: %s"),
                *InVoiceTask->GetLogName());
            InVoiceTask->Run(FVoiceTaskComplete::CreateLambda([OnDone]() {
                OnDone(true);
            }));
            return true;
        },
        [this](const TArray<bool> &OutResults) {
            UE_LOG(LogRedpointEOSVoiceChat, Verbose, TEXT("Finished synchronising voice chat state with EOS."));

            this->bIsSynchronising = false;

            // Kick off another sync operation if we need to.
            if (this->PendingSynchronisations != EEOSVoiceManagerLocalUserSyncMode::None)
            {
                this->PerformSynchronisationToEOS();
            }
        });
}

void FEOSVoiceManagerLocalUser::SetPlayersBlockState(const TArray<FString> &PlayerNames, bool bBlocked)
{
    using namespace ::Redpoint::EOS::API::RTC;

    for (const auto &PlayerName : PlayerNames)
    {
        TSharedPtr<const FUniqueNetIdEOS> ParticipantId = FUniqueNetIdEOS::ParseFromString(PlayerName);
        if (ParticipantId.IsValid())
        {
            for (const auto &Channel : this->JoinedChannels)
            {
                if (Channel.Value->Participants.Contains(*ParticipantId))
                {
                    FBlockParticipant::Execute(
                        this->PlatformHandle,
                        FBlockParticipant::Options{
                            this->LocalUserId->GetProductUserId(),
                            Channel.Value->RoomName,
                            ParticipantId->GetProductUserId(),
                            bBlocked},
                        FBlockParticipant::CompletionDelegate::CreateSPLambda(
                            this,
                            [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                bBlocked,
#endif
                                ParticipantId,
                                RoomName = Channel.Value->RoomName](const FBlockParticipant::Result &Result) {
                                if (Result.ResultCode != EOS_EResult::EOS_Success)
                                {
                                    UE_LOG(
                                        LogRedpointEOS,
                                        Warning,
                                        TEXT("EOS_RTC_BlockParticipant operation (%s) for remote user '%s' in room "
                                             "'%s' failed with result code %s"),
                                        bBlocked ? TEXT("blocking") : TEXT("unblocking"),
                                        *ParticipantId->ToString(),
                                        *RoomName,
                                        ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));
                                }
                            }));
                }
            }
        }
    }
}

void FEOSVoiceManagerLocalUser::SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode SyncMode)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChatUser, SyncScheduleSynchronisation);

    if (this->bIsSynchronising)
    {
        // Schedule another synchronisation after this one.
        this->PendingSynchronisations = (EEOSVoiceManagerLocalUserSyncMode)(this->PendingSynchronisations | SyncMode);
    }
    else if (this->bHasSynchronisedThisFrame)
    {
        // We've already synchronised in this frame. To prevent redundant no-op calls into the
        // EOS SDK if the game code is calling the voice chat API frequently, we only synchronise
        // at most once per frame, and then set a flag that we need to synchronise next frame instead.
        this->PendingSynchronisations = (EEOSVoiceManagerLocalUserSyncMode)(this->PendingSynchronisations | SyncMode);
        this->bSynchroniseOnNextFrame = true;
    }
    else
    {
        this->PendingSynchronisations = SyncMode;
        this->bHasSynchronisedThisFrame = true;
        this->PerformSynchronisationToEOS();
    }
}

void FEOSVoiceManagerLocalUser::RegisterLobbyChannel(
    const TSharedRef<const FOnlinePartyIdRedpointBase> &InLobbyId,
    const FString &ChannelName,
    EVoiceChatChannelType ChannelType)
{
    if (!this->JoinedChannels.Contains(ChannelName))
    {
        this->JoinedChannels.Add(
            ChannelName,
            FEOSVoiceManagerLocalUserJoinedChannel::NewLobby(
                this->AsShared(),
                this->PlatformHandle,
                ChannelName,
                InLobbyId,
                ChannelType));
    }
    else
    {
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("%s"),
            *Redpoint::EOS::API::Errors::DuplicateNotAllowed(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("The user was already in this chat channel by the time RegisterLobbyChannel "
                      "was called. The voice chat system state may be inconsistent."))
                 .ToLogString());
    }
    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::TransmitModeAndJoinedRoom);
    this->OnVoiceChatChannelJoined().Broadcast(ChannelName);
}

void FEOSVoiceManagerLocalUser::UnregisterLobbyChannel(const TSharedRef<const FOnlinePartyIdRedpointBase> &InLobbyId)
{
    FString ChannelName;
    for (const auto &KV : this->JoinedChannels)
    {
        if (KV.Value->LobbyId.IsValid() && *KV.Value->LobbyId == *InLobbyId)
        {
            ChannelName = KV.Value->RoomName;
            break;
        }
    }
    if (ChannelName.IsEmpty())
    {
        return;
    }

    if (this->JoinedChannels.Contains(ChannelName))
    {
        this->JoinedChannels.Remove(ChannelName);
        this->OnVoiceChatChannelExited().Broadcast(ChannelName, FVoiceChatResult(EVoiceChatResult::Success));
    }
}

void FEOSVoiceManagerLocalUser::OnAvailableAudioDevicesChanged()
{
    this->OnVoiceChatAvailableAudioDevicesChanged().Broadcast();
}

FEOSVoiceManagerLocalUser::FEOSVoiceManagerLocalUser(
    FPlatformHandle InPlatformHandle,
    const TSharedRef<FEOSVoiceManager> &InVoiceManager,
    const TSharedRef<const FUniqueNetIdEOS> &InLocalUserId)
    : PlatformHandle(InPlatformHandle)
    , VoiceManager(InVoiceManager)
    , LocalUserId(InLocalUserId)
    , Settings()
    , bHasSetCurrentInputDevice(false)
    , CurrentInputDevice()
    , bHasSetCurrentOutputDevice(false)
    , CurrentOutputDevice()
    , OnDevicesChangedHandle()
    , bTransmitToAllChannels(true)
    , TransmitChannelNames()
    , InputVolume(100.0f)
    , OutputVolume(100.0f)
    , bInputMuted(false)
    , bOutputMuted(false)
    , bHasSynchronisedThisFrame(false)
    , bSynchroniseOnNextFrame(false)
    , bIsSynchronising(false)
    , PendingSynchronisations(EEOSVoiceManagerLocalUserSyncMode::None)
    , OnVoiceChatAvailableAudioDevicesChangedDelegate()
    , OnVoiceChatChannelJoinedDelegate()
    , OnVoiceChatChannelExitedDelegate()
    , OnVoiceChatCallStatsUpdatedDelegate()
    , OnVoiceChatPlayerAddedDelegate()
    , OnVoiceChatPlayerRemovedDelegate()
    , OnVoiceChatPlayerTalkingUpdatedDelegate()
    , OnVoiceChatPlayerMuteUpdatedDelegate()
    , OnVoiceChatPlayerVolumeUpdatedDelegate()
{
    this->Settings.Add("EnableEchoCancellation", "true");
    this->Settings.Add("EnableNoiseSuppression", "true");
    this->Settings.Add("EnableAutoGainControl", "true");
    this->Settings.Add("EnableDtx", "true");
    this->Settings.Add(
        "EnablePlatformAEC",
        this->VoiceManager->Config->GetEnableVoiceChatPlatformAECByDefault() ? "true" : "false");
}

void FEOSVoiceManagerLocalUser::RegisterEvents()
{
    this->OnDevicesChangedHandle = this->VoiceManager->DeviceList->OnDevicesChanged().Add(
        FOnVoiceChatAvailableAudioDevicesChangedDelegate::FDelegate::CreateSP(
            this,
            &FEOSVoiceManagerLocalUser::OnAvailableAudioDevicesChanged));
}

FEOSVoiceManagerLocalUser::~FEOSVoiceManagerLocalUser()
{
    if (this->OnDevicesChangedHandle.IsValid())
    {
        this->VoiceManager->DeviceList->OnDevicesChanged().Remove(this->OnDevicesChangedHandle);
    }
}

void FEOSVoiceManagerLocalUser::PostLoginInit(const FEOSVoiceManagerLocalUserPostLoginComplete &OnPostLoginComplete)
{
    this->VoiceManager->DeviceList->WaitForInitialDevices(
        FEOSVoiceManagerDeviceListWaitForInitialDevices::CreateLambda([OnPostLoginComplete]() {
            OnPostLoginComplete.ExecuteIfBound();
        }));
}

void FEOSVoiceManagerLocalUser::SetSetting(const FString &Name, const FString &Value)
{
    this->Settings.Add(Name, Value);
    if (Name == "EnableEchoCancellation" || Name == "EnableNoiseSuppression" || Name == "EnableAutoGainControl" ||
        Name == "EnableDtx" || Name == "EnablePlatformAEC" || Name == "PlatformAECEnabled")
    {
        this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::InputSettings);
    }
}

FString FEOSVoiceManagerLocalUser::GetSetting(const FString &Name)
{
    // Serve lobby IDs as "__EOS_LobbyId:<channel name>" and "__EOS_PartyId:<channel name>"
    if (Name.StartsWith("__EOS_LobbyId:") || Name.StartsWith("__EOS_PartyId:"))
    {
        TArray<FString> Components;
        Name.ParseIntoArray(Components, TEXT(":"), true);
        if (Components.Num() >= 2)
        {
            for (const auto &KV : this->JoinedChannels)
            {
                if (KV.Key == Components[1])
                {
                    if (KV.Value->LobbyId.IsValid())
                    {
                        return KV.Value->LobbyId->ToString();
                    }
                    return TEXT("");
                }
            }
        }
    }

    if (this->Settings.Contains(Name))
    {
        return this->Settings[Name];
    }
    return TEXT("");
}

void FEOSVoiceManagerLocalUser::SetAudioInputVolume(float Volume)
{
    REDPOINT_EOS_SCOPE_CYCLE_COUNTER(VoiceChatUser, ApiSetAudioInputVolume);

    this->InputVolume = Volume;
    if (this->InputVolume < 0.0f)
    {
        this->InputVolume = 0.0f;
    }
    if (this->InputVolume > 200.0f)
    {
        this->InputVolume = 200.0f;
    }

    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::InputSettings);
}

void FEOSVoiceManagerLocalUser::SetAudioOutputVolume(float Volume)
{
    this->OutputVolume = Volume;
    if (this->OutputVolume < 0.0f)
    {
        this->OutputVolume = 0.0f;
    }
    if (this->OutputVolume > 200.0f)
    {
        this->OutputVolume = 200.0f;
    }

    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::OutputSettings);
}

float FEOSVoiceManagerLocalUser::GetAudioInputVolume() const
{
    return this->InputVolume;
}

float FEOSVoiceManagerLocalUser::GetAudioOutputVolume() const
{
    return this->OutputVolume;
}

void FEOSVoiceManagerLocalUser::SetAudioInputDeviceMuted(bool bIsMuted)
{
    this->bInputMuted = bIsMuted;

    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::InputSettings);
}

void FEOSVoiceManagerLocalUser::SetAudioOutputDeviceMuted(bool bIsMuted)
{
    this->bOutputMuted = bIsMuted;

    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::OutputSettings);
}

bool FEOSVoiceManagerLocalUser::GetAudioInputDeviceMuted() const
{
    return this->bInputMuted;
}

bool FEOSVoiceManagerLocalUser::GetAudioOutputDeviceMuted() const
{
    return this->bOutputMuted;
}

TArray<FEOSVoiceManagerDevice> FEOSVoiceManagerLocalUser::GetAvailableInputDeviceInfos() const
{
    return this->VoiceManager->DeviceList->GetInputDevices();
}

TArray<FEOSVoiceManagerDevice> FEOSVoiceManagerLocalUser::GetAvailableOutputDeviceInfos() const
{
    return this->VoiceManager->DeviceList->GetOutputDevices();
}

void FEOSVoiceManagerLocalUser::SetInputDeviceId(const FString &InputDeviceId)
{
    bool bFoundTargetInputDevice = false;
    FEOSVoiceManagerDevice TargetInputDeviceId;
    TArray<FEOSVoiceManagerDevice> Results = this->GetAvailableInputDeviceInfos();
    for (const auto &Result : Results)
    {
        if (Result.Id == InputDeviceId)
        {
            TargetInputDeviceId = Result;
            bFoundTargetInputDevice = true;
            break;
        }
    }
    if (!bFoundTargetInputDevice)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("Failed to set audio input options. No such audio input device."));
        return;
    }

    this->CurrentInputDevice = TargetInputDeviceId;
    this->bHasSetCurrentInputDevice = true;

    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::InputSettings);
}

void FEOSVoiceManagerLocalUser::SetOutputDeviceId(const FString &OutputDeviceId)
{
    bool bFoundTargetOutputDevice = false;
    FEOSVoiceManagerDevice TargetOutputDeviceId;
    TArray<FEOSVoiceManagerDevice> Results = this->GetAvailableOutputDeviceInfos();
    for (const auto &Result : Results)
    {
        if (Result.Id == OutputDeviceId)
        {
            TargetOutputDeviceId = Result;
            bFoundTargetOutputDevice = true;
            break;
        }
    }
    if (!bFoundTargetOutputDevice)
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("Failed to set audio output options. No such audio output device."));
        return;
    }

    this->CurrentOutputDevice = TargetOutputDeviceId;
    this->bHasSetCurrentOutputDevice = true;

    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::OutputSettings);
}

FEOSVoiceManagerDevice FEOSVoiceManagerLocalUser::GetInputDeviceInfo() const
{
    if (this->bHasSetCurrentInputDevice)
    {
        return this->CurrentInputDevice;
    }
    return this->GetDefaultInputDeviceInfo();
}

FEOSVoiceManagerDevice FEOSVoiceManagerLocalUser::GetOutputDeviceInfo() const
{
    if (this->bHasSetCurrentOutputDevice)
    {
        return this->CurrentOutputDevice;
    }
    return this->GetDefaultOutputDeviceInfo();
}

FEOSVoiceManagerDevice FEOSVoiceManagerLocalUser::GetDefaultInputDeviceInfo() const
{
    TArray<FEOSVoiceManagerDevice> Results = this->GetAvailableInputDeviceInfos();
    for (const auto &Result : Results)
    {
        if (Result.bIsDefaultDevice)
        {
            return Result;
        }
    }
    return FEOSVoiceManagerDevice();
}

FEOSVoiceManagerDevice FEOSVoiceManagerLocalUser::GetDefaultOutputDeviceInfo() const
{
    TArray<FEOSVoiceManagerDevice> Results = this->GetAvailableOutputDeviceInfos();
    for (const auto &Result : Results)
    {
        if (Result.bIsDefaultDevice)
        {
            return Result;
        }
    }
    return FEOSVoiceManagerDevice();
}

void FEOSVoiceManagerLocalUser::BlockPlayers(const TArray<FString> &PlayerNames)
{
    this->SetPlayersBlockState(PlayerNames, true);
}

void FEOSVoiceManagerLocalUser::UnblockPlayers(const TArray<FString> &PlayerNames)
{
    this->SetPlayersBlockState(PlayerNames, false);
}

void FEOSVoiceManagerLocalUser::JoinChannel(
    const FString &ChannelName,
    const FString &ChannelCredentials,
    EVoiceChatChannelType ChannelType,
    const FOnVoiceChatChannelJoinCompleteDelegate &Delegate,
    const TOptional<FVoiceChatChannel3dProperties> &Channel3dProperties)
{
    using namespace ::Redpoint::EOS::API::RTC;

    if (ChannelType == EVoiceChatChannelType::Positional)
    {
        Delegate.ExecuteIfBound(
            ChannelName,
            FVoiceChatResult(
                EVoiceChatResult::InvalidArgument,
                TEXT("PositionalChannelsNotSupported"),
                TEXT("EOS does not support positional voice chat channels yet.")));
        return;
    }

    if (this->JoinedChannels.Contains(ChannelName))
    {
        Delegate.ExecuteIfBound(
            ChannelName,
            FVoiceChatResult(
                EVoiceChatResult::InvalidArgument,
                TEXT("AlreadyJoined"),
                TEXT("This user is already connected to that voice chat channel.")));
        return;
    }

    FString ClientBaseUrl, ParticipantToken;
    ChannelCredentials
        .Split(TEXT("?token="), &ClientBaseUrl, &ParticipantToken, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

    FJoinRoom::Execute(
        this->PlatformHandle,
        FJoinRoom::Options{
            this->LocalUserId->GetProductUserId(),
            ChannelName,
            ClientBaseUrl,
            ParticipantToken,
            (EOS_ProductUserId) nullptr,
            (uint32)(ChannelType == EVoiceChatChannelType::Echo ? EOS_RTC_JOINROOMFLAGS_ENABLE_ECHO : 0x0),
            false,
            false},
        FJoinRoom::CompletionDelegate::CreateSPLambda(
            this,
            [this, Delegate, ChannelName, ChannelType](const FJoinRoom::Result &Result) {
                if (Result.ResultCode == EOS_EResult::EOS_Success)
                {
                    if (!this->JoinedChannels.Contains(ChannelName))
                    {
                        this->JoinedChannels.Add(
                            ChannelName,
                            FEOSVoiceManagerLocalUserJoinedChannel::NewDedicated(
                                this->AsShared(),
                                this->PlatformHandle,
                                ChannelName,
                                ChannelType));
                    }
                    else
                    {
                        UE_LOG(
                            LogRedpointEOS,
                            Warning,
                            TEXT("%s"),
                            *Redpoint::EOS::API::Errors::DuplicateNotAllowed(
                                 ANSI_TO_TCHAR(__FUNCTION__),
                                 TEXT("The user was already in this chat channel by the time EOS_RTC_JoinRoom "
                                      "completed successfully. The voice chat system state may be inconsistent."))
                                 .ToLogString());
                    }
                    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::TransmitModeAndJoinedRoom);
                    this->OnVoiceChatChannelJoined().Broadcast(ChannelName);
                    Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::Success));
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("Failed to join RTC channel: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));
                    if (Result.ResultCode == EOS_EResult::EOS_NoConnection)
                    {
                        Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::ConnectionFailure));
                    }
                    else if (Result.ResultCode == EOS_EResult::EOS_InvalidAuth)
                    {
                        Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::CredentialsInvalid));
                    }
                    else if (Result.ResultCode == EOS_EResult::EOS_RTC_TooManyParticipants)
                    {
                        Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::NotPermitted));
                    }
                    else if (Result.ResultCode == EOS_EResult::EOS_AccessDenied)
                    {
                        Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::InvalidArgument));
                    }
                    else
                    {
                        Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::ImplementationError));
                    }
                }
            }));
}

void FEOSVoiceManagerLocalUser::LeaveChannel(
    const FString &ChannelName,
    const FOnVoiceChatChannelLeaveCompleteDelegate &Delegate)
{
    using namespace ::Redpoint::EOS::API::RTC;

    FLeaveRoom::Execute(
        this->PlatformHandle,
        FLeaveRoom::Options{this->LocalUserId->GetProductUserId(), ChannelName},
        FLeaveRoom::CompletionDelegate::CreateSPLambda(
            this,
            [this, ChannelName, Delegate](const FLeaveRoom::Result &Result) {
                if (Result.ResultCode == EOS_EResult::EOS_Success)
                {
                    if (this->JoinedChannels.Contains(ChannelName))
                    {
                        this->JoinedChannels.Remove(ChannelName);
                        this->OnVoiceChatChannelExited().Broadcast(
                            ChannelName,
                            FVoiceChatResult(EVoiceChatResult::Success));
                    }
                    Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::Success));
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOS,
                        Error,
                        TEXT("Failed to leave RTC channel: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));
                    if (Result.ResultCode == EOS_EResult::EOS_AccessDenied)
                    {
                        Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::InvalidArgument));
                    }
                    else
                    {
                        Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult(EVoiceChatResult::ImplementationError));
                    }
                }
            }));
}

void FEOSVoiceManagerLocalUser::Set3DPosition(
    const FString &ChannelName,
    const FVector &SpeakerPosition,
    const FVector &ListenerPosition,
    const FVector &ListenerForwardDirection,
    const FVector &ListenerUpDirection)
{
    UE_LOG(
        LogRedpointEOS,
        Error,
        TEXT("EOS does not support calling Set3DPosition, because positional voice channels are not supported yet."));
}

TArray<FString> FEOSVoiceManagerLocalUser::GetChannels()
{
    TArray<FString> Keys;
    this->JoinedChannels.GenerateKeyArray(Keys);
    return Keys;
}

const TArray<FString> FEOSVoiceManagerLocalUser::GetPlayersInChannel(const FString &ChannelName) const
{
    TArray<FString> PlayerIds;
    if (this->JoinedChannels.Contains(ChannelName))
    {
        for (const auto &RemoteUser : this->JoinedChannels[ChannelName]->Participants)
        {
            PlayerIds.Add(RemoteUser.Key->ToString());
        }
    }
    return PlayerIds;
}

EVoiceChatChannelType FEOSVoiceManagerLocalUser::GetChannelType(const FString &ChannelName) const
{
    if (this->JoinedChannels.Contains(ChannelName))
    {
        return this->JoinedChannels[ChannelName]->ChannelType;
    }

    return EVoiceChatChannelType::NonPositional;
}

bool FEOSVoiceManagerLocalUser::IsPlayerTalking(const FString &PlayerName) const
{
    if (PlayerName == this->LocalUserId->ToString())
    {
        // If the local user is muted, they are never talking.
        if (this->bInputMuted || this->InputVolume == 0.0f)
        {
            return false;
        }
    }

    TSharedPtr<const FUniqueNetIdEOS> ParticipantId = FUniqueNetIdEOS::ParseFromString(PlayerName);
    if (ParticipantId.IsValid())
    {
        for (const auto &Channel : this->JoinedChannels)
        {
            if (Channel.Value->Participants.Contains(*ParticipantId))
            {
                if (Channel.Value->Participants[*ParticipantId]->bIsTalking)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void FEOSVoiceManagerLocalUser::SetPlayerMuted(const FString &PlayerName, bool bMuted)
{
    using namespace ::Redpoint::EOS::API::RTCAudio;

    TSharedPtr<const FUniqueNetIdEOS> ParticipantId = FUniqueNetIdEOS::ParseFromString(PlayerName);
    if (ParticipantId.IsValid())
    {
        if (*ParticipantId == *this->LocalUserId)
        {
            // We're trying to mute ourselves, not a remote player.
            this->SetAudioInputDeviceMuted(bMuted);
            return;
        }

        for (const auto &Channel : this->JoinedChannels)
        {
            if (Channel.Value->Participants.Contains(*ParticipantId))
            {
                FUpdateReceiving::Execute(
                    this->PlatformHandle,
                    FUpdateReceiving::Options{
                        this->LocalUserId->GetProductUserId(),
                        Channel.Value->RoomName,
                        ParticipantId->GetProductUserId(),
                        bMuted ? EAudioEnabled::No : EAudioEnabled::Yes},
                    FUpdateReceiving::CompletionDelegate::CreateSPLambda(
                        this,
                        [this, bMuted, ParticipantId, RoomName = Channel.Key, RoomInfo = Channel.Value](
                            const FUpdateReceiving::Result &Result) {
                            if (Result.ResultCode != EOS_EResult::EOS_Success)
                            {
                                UE_LOG(
                                    LogRedpointEOS,
                                    Warning,
                                    TEXT(
                                        "EOS_RTCAudio_UpdateReceiving operation (%s) for remote user '%s' in room '%s' "
                                        "failed with result code %s"),
                                    bMuted ? TEXT("muting") : TEXT("unmuting"),
                                    *ParticipantId->ToString(),
                                    *RoomName,
                                    ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));
                            }
                            else
                            {
                                // Update the bIsMuted flag.
                                if (RoomInfo->Participants.Contains(*ParticipantId))
                                {
                                    RoomInfo->Participants[*ParticipantId]->bIsMuted = bMuted;
                                    this->OnVoiceChatPlayerMuteUpdated().Broadcast(
                                        RoomName,
                                        ParticipantId->ToString(),
                                        bMuted);
                                }
                            }
                        }));
            }
        }
    }
}

bool FEOSVoiceManagerLocalUser::IsPlayerMuted(const FString &PlayerName) const
{
    TSharedPtr<const FUniqueNetIdEOS> ParticipantId = FUniqueNetIdEOS::ParseFromString(PlayerName);
    if (ParticipantId.IsValid())
    {
        if (*ParticipantId == *this->LocalUserId)
        {
            // We're trying to check if we're muted ourselves, not a remote player.
            return this->GetAudioInputDeviceMuted();
        }

        for (const auto &Channel : this->JoinedChannels)
        {
            if (Channel.Value->Participants.Contains(*ParticipantId))
            {
                if (Channel.Value->Participants[*ParticipantId]->bIsMuted)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void FEOSVoiceManagerLocalUser::SetPlayerVolume(const FString &PlayerName, float Volume)
{
    if (Volume == 0.0f)
    {
        this->SetPlayerMuted(PlayerName, true);
    }
    else
    {
        this->SetPlayerMuted(PlayerName, false);
    }
}

float FEOSVoiceManagerLocalUser::GetPlayerVolume(const FString &PlayerName) const
{
    return this->IsPlayerMuted(PlayerName) ? 0.0f : 100.0f;
}

void FEOSVoiceManagerLocalUser::TransmitToAllChannels()
{
    this->bTransmitToAllChannels = true;
    this->TransmitChannelNames.Empty();
    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::TransmitMode);
}

void FEOSVoiceManagerLocalUser::TransmitToNoChannels()
{
    this->bTransmitToAllChannels = false;
    this->TransmitChannelNames.Empty();
    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::TransmitMode);
}

#if REDPOINT_EOS_UE_5_3_OR_LATER
void FEOSVoiceManagerLocalUser::TransmitToSpecificChannels(const TSet<FString> &ChannelNames)
{
    this->bTransmitToAllChannels = false;
    this->TransmitChannelNames = ChannelNames;
    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::TransmitMode);
}
#else
void FEOSVoiceManagerLocalUser::TransmitToSpecificChannel(const FString &ChannelName)
{
    this->bTransmitToAllChannels = false;
    this->TransmitChannelNames.Add(ChannelName);
    this->SynchroniseToEOS(EEOSVoiceManagerLocalUserSyncMode::TransmitMode);
}
#endif

EVoiceChatTransmitMode FEOSVoiceManagerLocalUser::GetTransmitMode() const
{
    if (this->bTransmitToAllChannels)
    {
        return EVoiceChatTransmitMode::All;
    }
    else if (this->TransmitChannelNames.Num() == 0)
    {
        return EVoiceChatTransmitMode::None;
    }
    else
    {
#if REDPOINT_EOS_UE_5_3_OR_LATER
        return EVoiceChatTransmitMode::SpecificChannels;
#else
        return EVoiceChatTransmitMode::Channel;
#endif
    }
}

#if REDPOINT_EOS_UE_5_3_OR_LATER
TSet<FString> FEOSVoiceManagerLocalUser::GetTransmitChannels() const
{
    return this->TransmitChannelNames;
}
#else
FString FEOSVoiceManagerLocalUser::GetTransmitChannel() const
{
    if (this->bTransmitToAllChannels)
    {
        return TEXT("");
    }

    return FString::Join(this->TransmitChannelNames, TEXT(","));
}
#endif

FOnVoiceChatAvailableAudioDevicesChangedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatAvailableAudioDevicesChanged()
{
    return this->OnVoiceChatAvailableAudioDevicesChangedDelegate;
}

FOnVoiceChatChannelJoinedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatChannelJoined()
{
    return this->OnVoiceChatChannelJoinedDelegate;
}

FOnVoiceChatChannelExitedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatChannelExited()
{
    return this->OnVoiceChatChannelExitedDelegate;
}

FOnVoiceChatCallStatsUpdatedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatCallStatsUpdated()
{
    return this->OnVoiceChatCallStatsUpdatedDelegate;
}

FOnVoiceChatPlayerAddedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatPlayerAdded()
{
    return this->OnVoiceChatPlayerAddedDelegate;
}

FOnVoiceChatPlayerRemovedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatPlayerRemoved()
{
    return this->OnVoiceChatPlayerRemovedDelegate;
}

FOnVoiceChatPlayerTalkingUpdatedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatPlayerTalkingUpdated()
{
    return this->OnVoiceChatPlayerTalkingUpdatedDelegate;
}

FOnVoiceChatPlayerMuteUpdatedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatPlayerMuteUpdated()
{
    return this->OnVoiceChatPlayerMuteUpdatedDelegate;
}

FOnVoiceChatPlayerVolumeUpdatedDelegate &FEOSVoiceManagerLocalUser::OnVoiceChatPlayerVolumeUpdated()
{
    return this->OnVoiceChatPlayerVolumeUpdatedDelegate;
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()