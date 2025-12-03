// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/VoiceChatUser.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemUtils.h"

IVoiceChatUser *UVoiceChatUser::GetHandle()
{
    return this->HandlePtr;
}

bool UVoiceChatUser::IsHandleValid(IVoiceChatUser *InHandle)
{
    return InHandle != nullptr;
}

UVoiceChatUser::UVoiceChatUser()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        return;
    }

    this->bOwnsHandle = false;
    this->HandlePtr = nullptr;
}

void UVoiceChatUser::AssignUserHandle(IVoiceChatUser *InHandlePtr, bool bInOwnsHandle)
{
    checkf(this->HandlePtr == nullptr, TEXT("Expected HandlePtr to be nullptr!"));
    checkf(InHandlePtr != nullptr, TEXT("Expected InHandlePtr to not be nullptr!"));
    this->HandlePtr = InHandlePtr;
    this->bOwnsHandle = bInOwnsHandle;
    this->RegisterFunctionDelegates();
}

void UVoiceChatUser::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    if (this->HandlePtr != nullptr)
    {
        this->UnregisterFunctionDelegates();
    }
    if (this->bOwnsHandle && this->HandlePtr != nullptr)
    {
        IVoiceChat::Get()->ReleaseUser(this->HandlePtr);
    }
    this->bOwnsHandle = false;
    this->HandlePtr = nullptr;

    Super::BeginDestroy();
}
void UVoiceChatUser::RegisterFunctionDelegates()
{
    auto Handle = this->GetHandle();
    if (this->IsHandleValid(Handle))
    {
        this->FunctionDelegateHandle_FOnVoiceChatAvailableAudioDevicesChangedDelegate =
            Handle->OnVoiceChatAvailableAudioDevicesChanged().AddLambda([this]() {
                this->OnVoiceChatAvailableAudioDevicesChanged.Broadcast();
            });
        this->FunctionDelegateHandle_FOnVoiceChatLoggedInDelegate =
            Handle->OnVoiceChatLoggedIn().AddLambda([this](const FString &PlayerName) {
                this->OnVoiceChatLoggedIn.Broadcast(PlayerName);
            });
        this->FunctionDelegateHandle_FOnVoiceChatLoggedOutDelegate =
            Handle->OnVoiceChatLoggedOut().AddLambda([this](const FString &PlayerName) {
                this->OnVoiceChatLoggedOut.Broadcast(PlayerName);
            });
        this->FunctionDelegateHandle_FOnVoiceChatChannelJoinedDelegate =
            Handle->OnVoiceChatChannelJoined().AddLambda([this](const FString &ChannelName) {
                this->OnVoiceChatChannelJoined.Broadcast(ChannelName);
            });
        this->FunctionDelegateHandle_FOnVoiceChatChannelExitedDelegate = Handle->OnVoiceChatChannelExited().AddLambda(
            [this](const FString &ChannelName, const FVoiceChatResult &Reason) {
                this->OnVoiceChatChannelExited.Broadcast(ChannelName, FVoiceChatResultBP::FromNative(Reason));
            });
        this->FunctionDelegateHandle_FOnVoiceChatPlayerAddedDelegate =
            Handle->OnVoiceChatPlayerAdded().AddLambda([this](const FString &ChannelName, const FString &PlayerName) {
                this->OnVoiceChatPlayerAdded.Broadcast(ChannelName, PlayerName);
            });
        this->FunctionDelegateHandle_FOnVoiceChatPlayerRemovedDelegate =
            Handle->OnVoiceChatPlayerRemoved().AddLambda([this](const FString &ChannelName, const FString &PlayerName) {
                this->OnVoiceChatPlayerRemoved.Broadcast(ChannelName, PlayerName);
            });
        this->FunctionDelegateHandle_FOnVoiceChatPlayerTalkingUpdatedDelegate =
            Handle->OnVoiceChatPlayerTalkingUpdated().AddLambda(
                [this](const FString &ChannelName, const FString &PlayerName, bool bIsTalking) {
                    this->OnVoiceChatPlayerTalkingUpdated.Broadcast(ChannelName, PlayerName, bIsTalking);
                });
        this->FunctionDelegateHandle_FOnVoiceChatPlayerMuteUpdatedDelegate =
            Handle->OnVoiceChatPlayerMuteUpdated().AddLambda(
                [this](const FString &ChannelName, const FString &PlayerName, bool bIsMuted) {
                    this->OnVoiceChatPlayerMuteUpdated.Broadcast(ChannelName, PlayerName, bIsMuted);
                });
        this->FunctionDelegateHandle_FOnVoiceChatPlayerVolumeUpdatedDelegate =
            Handle->OnVoiceChatPlayerVolumeUpdated().AddLambda(
                [this](const FString &ChannelName, const FString &PlayerName, float Volume) {
                    this->OnVoiceChatPlayerVolumeUpdated.Broadcast(ChannelName, PlayerName, Volume);
                });
    }
}

void UVoiceChatUser::UnregisterFunctionDelegates()
{
    auto Handle = this->GetHandle();
    if (this->IsHandleValid(Handle))
    {
        Handle->OnVoiceChatAvailableAudioDevicesChanged().Remove(
            this->FunctionDelegateHandle_FOnVoiceChatAvailableAudioDevicesChangedDelegate);
        Handle->OnVoiceChatLoggedIn().Remove(this->FunctionDelegateHandle_FOnVoiceChatLoggedInDelegate);
        Handle->OnVoiceChatLoggedOut().Remove(this->FunctionDelegateHandle_FOnVoiceChatLoggedOutDelegate);
        Handle->OnVoiceChatChannelJoined().Remove(this->FunctionDelegateHandle_FOnVoiceChatChannelJoinedDelegate);
        Handle->OnVoiceChatChannelExited().Remove(this->FunctionDelegateHandle_FOnVoiceChatChannelExitedDelegate);
        Handle->OnVoiceChatPlayerAdded().Remove(this->FunctionDelegateHandle_FOnVoiceChatPlayerAddedDelegate);
        Handle->OnVoiceChatPlayerRemoved().Remove(this->FunctionDelegateHandle_FOnVoiceChatPlayerRemovedDelegate);
        Handle->OnVoiceChatPlayerTalkingUpdated().Remove(
            this->FunctionDelegateHandle_FOnVoiceChatPlayerTalkingUpdatedDelegate);
        Handle->OnVoiceChatPlayerMuteUpdated().Remove(
            this->FunctionDelegateHandle_FOnVoiceChatPlayerMuteUpdatedDelegate);
        Handle->OnVoiceChatPlayerVolumeUpdated().Remove(
            this->FunctionDelegateHandle_FOnVoiceChatPlayerVolumeUpdatedDelegate);
    }
}

void UVoiceChatUser::SetSetting(const FString &Name, const FString &Value)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetSetting(Name, Value);
}

FString UVoiceChatUser::GetSetting(const FString &Name)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TEXT("");
    }

    auto __Result = Handle->GetSetting(Name);

    return __Result;
}

void UVoiceChatUser::SetAudioInputVolume(float Volume)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetAudioInputVolume(Volume);
}

void UVoiceChatUser::SetAudioOutputVolume(float Volume)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetAudioOutputVolume(Volume);
}

float UVoiceChatUser::GetAudioInputVolume()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return 0;
    }

    auto __Result = Handle->GetAudioInputVolume();

    return __Result;
}

float UVoiceChatUser::GetAudioOutputVolume()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return 0;
    }

    auto __Result = Handle->GetAudioOutputVolume();

    return __Result;
}

void UVoiceChatUser::SetAudioInputDeviceMuted(bool bIsMuted)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetAudioInputDeviceMuted(bIsMuted);
}

void UVoiceChatUser::SetAudioOutputDeviceMuted(bool bIsMuted)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetAudioOutputDeviceMuted(bIsMuted);
}

bool UVoiceChatUser::GetAudioInputDeviceMuted()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    auto __Result = Handle->GetAudioInputDeviceMuted();

    return __Result;
}

bool UVoiceChatUser::GetAudioOutputDeviceMuted()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    auto __Result = Handle->GetAudioOutputDeviceMuted();

    return __Result;
}

TArray<FVoiceChatDeviceInfoBP> UVoiceChatUser::GetAvailableInputDeviceInfos()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TArray<FVoiceChatDeviceInfoBP>();
    }

    auto __Result = ConvertArrayElements<FVoiceChatDeviceInfo, FVoiceChatDeviceInfoBP>(
        Handle->GetAvailableInputDeviceInfos(),
        [](const FVoiceChatDeviceInfo &Val) {
            return FVoiceChatDeviceInfoBP::FromNative(Val);
        });

    return __Result;
}

TArray<FVoiceChatDeviceInfoBP> UVoiceChatUser::GetAvailableOutputDeviceInfos()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TArray<FVoiceChatDeviceInfoBP>();
    }

    auto __Result = ConvertArrayElements<FVoiceChatDeviceInfo, FVoiceChatDeviceInfoBP>(
        Handle->GetAvailableOutputDeviceInfos(),
        [](const FVoiceChatDeviceInfo &Val) {
            return FVoiceChatDeviceInfoBP::FromNative(Val);
        });

    return __Result;
}

void UVoiceChatUser::SetInputDeviceId(const FString &InputDeviceId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetInputDeviceId(InputDeviceId);
}

void UVoiceChatUser::SetOutputDeviceId(const FString &OutputDeviceId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetOutputDeviceId(OutputDeviceId);
}

FVoiceChatDeviceInfoBP UVoiceChatUser::GetInputDeviceInfo()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FVoiceChatDeviceInfoBP();
    }

    auto __Result = FVoiceChatDeviceInfoBP::FromNative(Handle->GetInputDeviceInfo());

    return __Result;
}

FVoiceChatDeviceInfoBP UVoiceChatUser::GetOutputDeviceInfo()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FVoiceChatDeviceInfoBP();
    }

    auto __Result = FVoiceChatDeviceInfoBP::FromNative(Handle->GetOutputDeviceInfo());

    return __Result;
}

FVoiceChatDeviceInfoBP UVoiceChatUser::GetDefaultInputDeviceInfo()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FVoiceChatDeviceInfoBP();
    }

    auto __Result = FVoiceChatDeviceInfoBP::FromNative(Handle->GetDefaultInputDeviceInfo());

    return __Result;
}

FVoiceChatDeviceInfoBP UVoiceChatUser::GetDefaultOutputDeviceInfo()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FVoiceChatDeviceInfoBP();
    }

    auto __Result = FVoiceChatDeviceInfoBP::FromNative(Handle->GetDefaultOutputDeviceInfo());

    return __Result;
}

UVoiceChatUserLogin *UVoiceChatUserLogin::Login(
    UVoiceChatUser *Subsystem,
    int32 PlatformId,
    const FString &PlayerName,
    const FString &Credentials)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(
            LogOnlineBlueprint,
            Error,
            TEXT("VoiceChatUser is not available. It's possible the online subsystem you are using doesn't support it. "
                 "Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the "
                 "Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your "
                 "logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UVoiceChatUserLogin *Node = NewObject<UVoiceChatUserLogin>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__PlatformId = PlatformId;
    Node->__Store__PlayerName = PlayerName;
    Node->__Store__Credentials = Credentials;
    return Node;
}

void UVoiceChatUserLogin::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }
    auto __DelegateHandle_OnVoiceChatLoginComplete = FOnVoiceChatLoginCompleteDelegate::CreateUObject(
        this,
        &UVoiceChatUserLogin::HandleCallback_OnVoiceChatLoginComplete);

    Handle->Login(
        BlueprintTypeToPlatformUserId(this->__Store__PlatformId),
        this->__Store__PlayerName,
        this->__Store__Credentials,
        __DelegateHandle_OnVoiceChatLoginComplete);
}

void UVoiceChatUserLogin::HandleCallback_OnVoiceChatLoginComplete(
    const FString &PlayerName,
    const FVoiceChatResult &Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }

    this->OnVoiceChatLoginComplete.Broadcast(PlayerName, FVoiceChatResultBP::FromNative(Result));
}

UVoiceChatUserLogout *UVoiceChatUserLogout::Logout(UVoiceChatUser *Subsystem)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(
            LogOnlineBlueprint,
            Error,
            TEXT("VoiceChatUser is not available. It's possible the online subsystem you are using doesn't support it. "
                 "Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the "
                 "Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your "
                 "logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UVoiceChatUserLogout *Node = NewObject<UVoiceChatUserLogout>();
    Node->__Store__Subsystem = Subsystem;
    return Node;
}

void UVoiceChatUserLogout::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }
    auto __DelegateHandle_OnVoiceChatLogoutComplete = FOnVoiceChatLogoutCompleteDelegate::CreateUObject(
        this,
        &UVoiceChatUserLogout::HandleCallback_OnVoiceChatLogoutComplete);

    Handle->Logout(__DelegateHandle_OnVoiceChatLogoutComplete);
}

void UVoiceChatUserLogout::HandleCallback_OnVoiceChatLogoutComplete(
    const FString &PlayerName,
    const FVoiceChatResult &Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }

    this->OnVoiceChatLogoutComplete.Broadcast(PlayerName, FVoiceChatResultBP::FromNative(Result));
}

bool UVoiceChatUser::IsLoggingIn()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    auto __Result = Handle->IsLoggingIn();

    return __Result;
}

bool UVoiceChatUser::IsLoggedIn()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    auto __Result = Handle->IsLoggedIn();

    return __Result;
}

FString UVoiceChatUser::GetLoggedInPlayerName()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TEXT("");
    }

    auto __Result = Handle->GetLoggedInPlayerName();

    return __Result;
}

void UVoiceChatUser::BlockPlayers(const TArray<FString> &PlayerNames)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }
    if (!(ValidateArrayElements<FString>(PlayerNames, [](const FString &Val) {
            return true;
        })))
    {
        return;
    }

    Handle->BlockPlayers(ConvertArrayElements<FString, FString>(PlayerNames, [](const FString &Val) {
        return Val;
    }));
}

void UVoiceChatUser::UnblockPlayers(const TArray<FString> &PlayerNames)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }
    if (!(ValidateArrayElements<FString>(PlayerNames, [](const FString &Val) {
            return true;
        })))
    {
        return;
    }

    Handle->UnblockPlayers(ConvertArrayElements<FString, FString>(PlayerNames, [](const FString &Val) {
        return Val;
    }));
}

UVoiceChatUserJoinChannel *UVoiceChatUserJoinChannel::JoinChannel(
    UVoiceChatUser *Subsystem,
    const FString &ChannelName,
    const FString &ChannelCredentials,
    EVoiceChatChannelType_ ChannelType,
    const FVoiceChatChannel3dPropertiesBP &Channel3dProperties)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(
            LogOnlineBlueprint,
            Error,
            TEXT("VoiceChatUser is not available. It's possible the online subsystem you are using doesn't support it. "
                 "Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the "
                 "Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your "
                 "logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UVoiceChatUserJoinChannel *Node = NewObject<UVoiceChatUserJoinChannel>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__ChannelName = ChannelName;
    Node->__Store__ChannelCredentials = ChannelCredentials;
    Node->__Store__ChannelType = ChannelType;
    Node->__Store__Channel3dProperties = Channel3dProperties;
    return Node;
}

void UVoiceChatUserJoinChannel::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }
    auto __DelegateHandle_OnVoiceChatChannelJoinComplete = FOnVoiceChatChannelJoinCompleteDelegate::CreateUObject(
        this,
        &UVoiceChatUserJoinChannel::HandleCallback_OnVoiceChatChannelJoinComplete);

    Handle->JoinChannel(
        this->__Store__ChannelName,
        this->__Store__ChannelCredentials,
        (EVoiceChatChannelType)(this->__Store__ChannelType),
        __DelegateHandle_OnVoiceChatChannelJoinComplete,
        this->__Store__Channel3dProperties.ToNative());
}

void UVoiceChatUserJoinChannel::HandleCallback_OnVoiceChatChannelJoinComplete(
    const FString &ChannelName,
    const FVoiceChatResult &Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }

    this->OnVoiceChatChannelJoinComplete.Broadcast(ChannelName, FVoiceChatResultBP::FromNative(Result));
}

UVoiceChatUserLeaveChannel *UVoiceChatUserLeaveChannel::LeaveChannel(
    UVoiceChatUser *Subsystem,
    const FString &ChannelName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(
            LogOnlineBlueprint,
            Error,
            TEXT("VoiceChatUser is not available. It's possible the online subsystem you are using doesn't support it. "
                 "Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the "
                 "Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your "
                 "logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UVoiceChatUserLeaveChannel *Node = NewObject<UVoiceChatUserLeaveChannel>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__ChannelName = ChannelName;
    return Node;
}

void UVoiceChatUserLeaveChannel::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }
    auto __DelegateHandle_OnVoiceChatChannelLeaveComplete = FOnVoiceChatChannelLeaveCompleteDelegate::CreateUObject(
        this,
        &UVoiceChatUserLeaveChannel::HandleCallback_OnVoiceChatChannelLeaveComplete);

    Handle->LeaveChannel(this->__Store__ChannelName, __DelegateHandle_OnVoiceChatChannelLeaveComplete);
}

void UVoiceChatUserLeaveChannel::HandleCallback_OnVoiceChatChannelLeaveComplete(
    const FString &ChannelName,
    const FVoiceChatResult &Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""), FVoiceChatResultBP());
        return;
    }

    this->OnVoiceChatChannelLeaveComplete.Broadcast(ChannelName, FVoiceChatResultBP::FromNative(Result));
}

void UVoiceChatUser::Set3DPosition(const FString &ChannelName, FVector SpeakerPosition)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

#if REDPOINT_EOS_UE_5_6_OR_LATER
    Handle->Set3DPosition(ChannelName, SpeakerPosition);
#else
    Handle->Set3DPosition(ChannelName, SpeakerPosition, FVector::ZeroVector, FVector::ZeroVector, FVector::ZeroVector);
#endif
}

TArray<FString> UVoiceChatUser::GetChannels()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TArray<FString>();
    }

    auto __Result = ConvertArrayElements<FString, FString>(Handle->GetChannels(), [](const FString &Val) {
        return Val;
    });

    return __Result;
}

TArray<FString> UVoiceChatUser::GetPlayersInChannel(const FString &ChannelName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TArray<FString>();
    }

    auto __Result =
        ConvertArrayElements<FString, FString>(Handle->GetPlayersInChannel(ChannelName), [](const FString &Val) {
            return Val;
        });

    return __Result;
}

EVoiceChatChannelType_ UVoiceChatUser::GetChannelType(const FString &ChannelName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return EVoiceChatChannelType_();
    }

    auto __Result = (EVoiceChatChannelType_)(Handle->GetChannelType(ChannelName));

    return __Result;
}

bool UVoiceChatUser::IsPlayerTalking(const FString &PlayerName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    auto __Result = Handle->IsPlayerTalking(PlayerName);

    return __Result;
}

void UVoiceChatUser::SetPlayerMuted(const FString &PlayerName, bool bMuted)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetPlayerMuted(PlayerName, bMuted);
}

bool UVoiceChatUser::IsPlayerMuted(const FString &PlayerName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    auto __Result = Handle->IsPlayerMuted(PlayerName);

    return __Result;
}

void UVoiceChatUser::SetChannelPlayerMuted(const FString &ChannelName, const FString &PlayerName, bool bAudioMuted)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetChannelPlayerMuted(ChannelName, PlayerName, bAudioMuted);
}

bool UVoiceChatUser::IsChannelPlayerMuted(const FString &ChannelName, const FString &PlayerName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    auto __Result = Handle->IsChannelPlayerMuted(ChannelName, PlayerName);

    return __Result;
}

void UVoiceChatUser::SetPlayerVolume(const FString &PlayerName, float Volume)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->SetPlayerVolume(PlayerName, Volume);
}

float UVoiceChatUser::GetPlayerVolume(const FString &PlayerName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return 0;
    }

    auto __Result = Handle->GetPlayerVolume(PlayerName);

    return __Result;
}

void UVoiceChatUser::TransmitToAllChannels()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->TransmitToAllChannels();
}

void UVoiceChatUser::TransmitToNoChannels()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }

    Handle->TransmitToNoChannels();
}

void UVoiceChatUser::TransmitToSpecificChannels(const TSet<FString> &ChannelNames)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return;
    }
    if (!(ValidateSetElements<FString>(ChannelNames, [](const FString &Val) {
            return true;
        })))
    {
        return;
    }

    Handle->TransmitToSpecificChannels(ConvertSetElements<FString, FString>(ChannelNames, [](const FString &Val) {
        return Val;
    }));
}

EVoiceChatTransmitMode_ UVoiceChatUser::GetTransmitMode()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return EVoiceChatTransmitMode_();
    }

    auto __Result = (EVoiceChatTransmitMode_)(Handle->GetTransmitMode());

    return __Result;
}

TSet<FString> UVoiceChatUser::GetTransmitChannels()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TSet<FString>();
    }

    auto __Result = ConvertSetElements<FString, FString>(Handle->GetTransmitChannels(), [](const FString &Val) {
        return Val;
    });

    return __Result;
}

FString UVoiceChatUser::InsecureGetLoginToken(const FString &PlayerName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TEXT("");
    }

    auto __Result = Handle->InsecureGetLoginToken(PlayerName);

    return __Result;
}

FString UVoiceChatUser::InsecureGetJoinToken(
    const FString &ChannelName,
    EVoiceChatChannelType_ ChannelType,
    const FVoiceChatChannel3dPropertiesBP &Channel3dProperties)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TEXT("");
    }

    auto __Result =
        Handle->InsecureGetJoinToken(ChannelName, (EVoiceChatChannelType)(ChannelType), Channel3dProperties.ToNative());

    return __Result;
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS