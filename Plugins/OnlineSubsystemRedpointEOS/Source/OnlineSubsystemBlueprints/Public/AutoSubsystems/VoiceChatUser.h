// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

#include "Delegates/Delegate.h"
#include "Delegates/DelegateCombinations.h"
#include "Engine/GameInstance.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Helpers/ArrayConversion.h"
#include "Helpers/UniqueNetIdConversion.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/OSBAchievementTypes.h"
#include "Types/OSBBlockedQueryResultInfo.h"
#include "Types/OSBChatTypes.h"
#include "Types/OSBEntitlementTypes.h"
#include "Types/OSBExternalUITypes.h"
#include "Types/OSBFileTypes.h"
#include "Types/OSBFriendTypes.h"
#include "Types/OSBGameActivityTypes.h"
#include "Types/OSBIdentityTypes.h"
#include "Types/OSBLeaderboardTypes.h"
#include "Types/OSBLobbyTypes.h"
#include "Types/OSBMessageTypes.h"
#include "Types/OSBOnlineAccountCredential.h"
#include "Types/OSBOnlineErrorInfo.h"
#include "Types/OSBOnlineLoginStatus.h"
#include "Types/OSBOnlineRecentPlayerRef.h"
#include "Types/OSBOnlineUserPrivilege.h"
#include "Types/OSBOnlineUserRef.h"
#include "Types/OSBPartyTypes.h"
#include "Types/OSBPurchaseTypes.h"
#include "Types/OSBSessionTypes.h"
#include "Types/OSBStatsTypes.h"
#include "Types/OSBStoreV2Types.h"
#include "Types/OSBVoiceChatTypes.h"

#include "VoiceChatUser.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoiceChatUser_OnVoiceChatAvailableAudioDevicesChangedDelegate_BP);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUser_OnVoiceChatLoggedInDelegate_BP, const FString &, PlayerName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FVoiceChatUser_OnVoiceChatLoggedOutDelegate_BP,
    const FString &,
    PlayerName);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUser_OnVoiceChatCallStatsUpdatedDelegate_BP, const
// FVoiceChatCallStats&, CallStats);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FVoiceChatUser_OnVoiceChatChannelJoinedDelegate_BP,
    const FString &,
    ChannelName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FVoiceChatUser_OnVoiceChatChannelExitedDelegate_BP,
    const FString &,
    ChannelName,
    const FVoiceChatResultBP &,
    Reason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FVoiceChatUser_OnVoiceChatPlayerAddedDelegate_BP,
    const FString &,
    ChannelName,
    const FString &,
    PlayerName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FVoiceChatUser_OnVoiceChatPlayerRemovedDelegate_BP,
    const FString &,
    ChannelName,
    const FString &,
    PlayerName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FVoiceChatUser_OnVoiceChatPlayerTalkingUpdatedDelegate_BP,
    const FString &,
    ChannelName,
    const FString &,
    PlayerName,
    bool,
    bIsTalking);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FVoiceChatUser_OnVoiceChatPlayerMuteUpdatedDelegate_BP,
    const FString &,
    ChannelName,
    const FString &,
    PlayerName,
    bool,
    bIsMuted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FVoiceChatUser_OnVoiceChatPlayerVolumeUpdatedDelegate_BP,
    const FString &,
    ChannelName,
    const FString &,
    PlayerName,
    float,
    Volume);

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUser : public UObject
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    FDelegateHandle DelegateHandle_FOnVoiceChatAvailableAudioDevicesChangedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatLoggedInDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatLoggedOutDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatChannelJoinedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatChannelExitedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerAddedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerRemovedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerTalkingUpdatedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerMuteUpdatedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerVolumeUpdatedDelegate;
    void RegisterFunctionDelegates();
    void UnregisterFunctionDelegates();

public:
private:
    bool bOwnsHandle;
    IVoiceChatUser *HandlePtr;
    IVoiceChatUser *GetHandle();
    bool IsHandleValid(IVoiceChatUser *InHandle);

public:
    UVoiceChatUser();
    void AssignUserHandle(IVoiceChatUser *InHandlePtr, bool bInOwnsHandle);
    virtual void BeginDestroy() override;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatAvailableAudioDevicesChangedDelegate_BP OnVoiceChatAvailableAudioDevicesChanged;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatAvailableAudioDevicesChangedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatLoggedInDelegate_BP OnVoiceChatLoggedIn;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatLoggedInDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatLoggedOutDelegate_BP OnVoiceChatLoggedOut;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatLoggedOutDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatChannelJoinedDelegate_BP OnVoiceChatChannelJoined;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatChannelJoinedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatChannelExitedDelegate_BP OnVoiceChatChannelExited;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatChannelExitedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerAddedDelegate_BP OnVoiceChatPlayerAdded;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerAddedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerRemovedDelegate_BP OnVoiceChatPlayerRemoved;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerRemovedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerTalkingUpdatedDelegate_BP OnVoiceChatPlayerTalkingUpdated;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerTalkingUpdatedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerMuteUpdatedDelegate_BP OnVoiceChatPlayerMuteUpdated;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerMuteUpdatedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerVolumeUpdatedDelegate_BP OnVoiceChatPlayerVolumeUpdated;

    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerVolumeUpdatedDelegate;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetSetting(const FString &Name, const FString &Value);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    FString GetSetting(const FString &Name);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetAudioInputVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetAudioOutputVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    float GetAudioInputVolume();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    float GetAudioOutputVolume();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetAudioInputDeviceMuted(bool bIsMuted);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetAudioOutputDeviceMuted(bool bIsMuted);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool GetAudioInputDeviceMuted();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool GetAudioOutputDeviceMuted();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    TArray<FVoiceChatDeviceInfoBP> GetAvailableInputDeviceInfos();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    TArray<FVoiceChatDeviceInfoBP> GetAvailableOutputDeviceInfos();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetInputDeviceId(const FString &InputDeviceId);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetOutputDeviceId(const FString &OutputDeviceId);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FVoiceChatDeviceInfoBP GetInputDeviceInfo();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FVoiceChatDeviceInfoBP GetOutputDeviceInfo();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FVoiceChatDeviceInfoBP GetDefaultInputDeviceInfo();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FVoiceChatDeviceInfoBP GetDefaultOutputDeviceInfo();

    friend class UVoiceChatUserLogin;

    friend class UVoiceChatUserLogout;

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsLoggingIn();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsLoggedIn();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FString GetLoggedInPlayerName();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void BlockPlayers(const TArray<FString> &PlayerNames);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void UnblockPlayers(const TArray<FString> &PlayerNames);

    friend class UVoiceChatUserJoinChannel;

    friend class UVoiceChatUserLeaveChannel;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void Set3DPosition(const FString &ChannelName, FVector SpeakerPosition);

    // @generator-duplicate-needs-picking
    // void Set3DPosition(const FString& ChannelName, const FVector& Position);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    TArray<FString> GetChannels();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    TArray<FString> GetPlayersInChannel(const FString &ChannelName);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    EVoiceChatChannelType_ GetChannelType(const FString &ChannelName);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsPlayerTalking(const FString &PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetPlayerMuted(const FString &PlayerName, bool bMuted);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsPlayerMuted(const FString &PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetChannelPlayerMuted(const FString &ChannelName, const FString &PlayerName, bool bAudioMuted);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsChannelPlayerMuted(const FString &ChannelName, const FString &PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetPlayerVolume(const FString &PlayerName, float Volume);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    float GetPlayerVolume(const FString &PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void TransmitToAllChannels();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void TransmitToNoChannels();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void TransmitToSpecificChannels(const TSet<FString> &ChannelNames);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    EVoiceChatTransmitMode_ GetTransmitMode();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    TSet<FString> GetTransmitChannels();

    // @generator-incompatible
    // FDelegateHandle StartRecording(const FOnVoiceChatRecordSamplesAvailableDelegate::FDelegate& Delegate);

    // @generator-incompatible
    // void StopRecording(FDelegateHandle Handle);

    // @generator-incompatible
    // FDelegateHandle RegisterOnVoiceChatAfterCaptureAudioReadDelegate(const
    // FOnVoiceChatAfterCaptureAudioReadDelegate::FDelegate& Delegate);

    // @generator-incompatible
    // void UnregisterOnVoiceChatAfterCaptureAudioReadDelegate(FDelegateHandle Handle);

    // @generator-incompatible
    // FDelegateHandle RegisterOnVoiceChatBeforeCaptureAudioSentDelegate(const
    // FOnVoiceChatBeforeCaptureAudioSentDelegate::FDelegate& Delegate);

    // @generator-incompatible
    // void UnregisterOnVoiceChatBeforeCaptureAudioSentDelegate(FDelegateHandle Handle);

    // @generator-incompatible
    // FDelegateHandle RegisterOnVoiceChatBeforeRecvAudioRenderedDelegate(const
    // FOnVoiceChatBeforeRecvAudioRenderedDelegate::FDelegate& Delegate);

    // @generator-incompatible
    // void UnregisterOnVoiceChatBeforeRecvAudioRenderedDelegate(FDelegateHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    FString InsecureGetLoginToken(const FString &PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    FString InsecureGetJoinToken(
        const FString &ChannelName,
        EVoiceChatChannelType_ ChannelType,
        const FVoiceChatChannel3dPropertiesBP &Channel3dProperties);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FVoiceChatUserLoginCallbackPin,
    const FString &,
    PlayerName,
    const FVoiceChatResultBP &,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserLogin : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLoginCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLoginCallbackPin OnVoiceChatLoginComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserLogin *Login(
        UVoiceChatUser *Subsystem,
        int32 PlatformId,
        const FString &PlayerName,
        const FString &Credentials);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UVoiceChatUser> __Store__Subsystem;
    void HandleCallback_OnVoiceChatLoginComplete(const FString &PlayerName, const FVoiceChatResult &Result);

    UPROPERTY()
    int32 __Store__PlatformId;

    UPROPERTY()
    FString __Store__PlayerName;

    UPROPERTY()
    FString __Store__Credentials;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FVoiceChatUserLogoutCallbackPin,
    const FString &,
    PlayerName,
    const FVoiceChatResultBP &,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserLogout : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLogoutCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLogoutCallbackPin OnVoiceChatLogoutComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserLogout *Logout(UVoiceChatUser *Subsystem

    );

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UVoiceChatUser> __Store__Subsystem;
    void HandleCallback_OnVoiceChatLogoutComplete(const FString &PlayerName, const FVoiceChatResult &Result);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FVoiceChatUserJoinChannelCallbackPin,
    const FString &,
    ChannelName,
    const FVoiceChatResultBP &,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserJoinChannel : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserJoinChannelCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserJoinChannelCallbackPin OnVoiceChatChannelJoinComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserJoinChannel *JoinChannel(
        UVoiceChatUser *Subsystem,
        const FString &ChannelName,
        const FString &ChannelCredentials,
        EVoiceChatChannelType_ ChannelType,
        const FVoiceChatChannel3dPropertiesBP &Channel3dProperties);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UVoiceChatUser> __Store__Subsystem;
    void HandleCallback_OnVoiceChatChannelJoinComplete(const FString &ChannelName, const FVoiceChatResult &Result);

    UPROPERTY()
    FString __Store__ChannelName;

    UPROPERTY()
    FString __Store__ChannelCredentials;

    UPROPERTY()
    EVoiceChatChannelType_ __Store__ChannelType;

    UPROPERTY()
    FVoiceChatChannel3dPropertiesBP __Store__Channel3dProperties;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FVoiceChatUserLeaveChannelCallbackPin,
    const FString &,
    ChannelName,
    const FVoiceChatResultBP &,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserLeaveChannel : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLeaveChannelCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLeaveChannelCallbackPin OnVoiceChatChannelLeaveComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserLeaveChannel *LeaveChannel(UVoiceChatUser *Subsystem, const FString &ChannelName);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UVoiceChatUser> __Store__Subsystem;
    void HandleCallback_OnVoiceChatChannelLeaveComplete(const FString &ChannelName, const FVoiceChatResult &Result);

    UPROPERTY()
    FString __Store__ChannelName;
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS