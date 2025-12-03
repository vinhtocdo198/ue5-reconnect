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
#include "Interfaces/OnlineIdentityInterface.h"
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

#include "OnlineIdentitySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentity_LoginChanged_BP, int32, LocalUserNum);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FIdentity_LoginStatusChanged_BP,
    int32,
    LocalUserNum,
    EOnlineLoginStatus,
    OldStatus,
    EOnlineLoginStatus,
    NewStatus,
    const FUniqueNetIdRepl &,
    NewId);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FIdentity_ControllerPairingChanged_BP, int, LocalUserNum,
// FControllerPairingChangedUserInfo, PreviousUser, FControllerPairingChangedUserInfo, NewUser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FIdentity_LoginComplete_BP,
    int32,
    LocalUserNum,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    Error);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIdentity_LogoutComplete_BP, int32, LocalUserNum, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentity_LoginFlowLogout_BP, const TArray<FString> &, LoginDomains);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlineIdentity, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineIdentity, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineIdentity, ESPMode::ThreadSafe> &InHandle);
    FDelegateHandle DelegateHandle_OnLoginChanged;
    TArray<FDelegateHandle> DelegateHandle_OnLoginStatusChanged;
    TArray<FDelegateHandle> DelegateHandle_OnLoginComplete;
    TArray<FDelegateHandle> DelegateHandle_OnLogoutComplete;
    FDelegateHandle DelegateHandle_OnLoginFlowLogout;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlineIdentitySubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LoginChanged_BP OnLoginChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LoginStatusChanged_BP OnLoginStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LoginComplete_BP OnLoginComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LogoutComplete_BP OnLogoutComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LoginFlowLogout_BP OnLoginFlowLogout;

    friend class UOnlineIdentitySubsystemLogin;

    friend class UOnlineIdentitySubsystemLogout;

    friend class UOnlineIdentitySubsystemAutoLogin;

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    UUserOnlineAccountRef *GetUserAccount(const FUniqueNetIdRepl &UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|Identity")
    TArray<UUserOnlineAccountRef *> GetAllUserAccounts();

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FUniqueNetIdRepl GetUniquePlayerId(int32 LocalUserNum);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FUniqueNetIdRepl GetSponsorUniquePlayerId(int32 LocalUserNum);

    UFUNCTION(BlueprintCallable, Category = "Online|Identity")
    FUniqueNetIdRepl CreateUniquePlayerId(const FString &Str);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    EOnlineLoginStatus GetLoginStatus(int32 LocalUserNum);

    // @generator-excluded-with-pick
    // ELoginStatus::Type GetLoginStatus(const FUniqueNetId& UserId);

    // @generator-excluded-with-pick
    // FString GetPlayerNickname(int32 LocalUserNum);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FString GetPlayerNickname(const FUniqueNetIdRepl &UserId);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FString GetAuthToken(int32 LocalUserNum);

    friend class UOnlineIdentitySubsystemRevokeAuthToken;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity")
    void ClearCachedAuthToken(const FUniqueNetIdRepl &UserId);

    // @result-delegates-not-compatible: FOnGetLinkedAccountAuthTokenCompleteDelegate(int32 LocalUserNum, bool
    // bWasSuccessful, const FExternalAuthToken& AuthToken) void GetLinkedAccountAuthToken(int32 LocalUserNum, const
    // FString& TokenType);

    friend class UOnlineIdentitySubsystemGetUserPrivilege;

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    int32 GetPlatformUserIdFromUniqueNetId(const FUniqueNetIdRepl &UniqueNetId);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FString GetAuthType();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlineIdentitySubsystemLoginCallbackPin,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemLogin : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemLoginCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemLoginCallbackPin OnLoginComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemLogin *Login(
        UOnlineIdentitySubsystem *Subsystem,
        int32 LocalUserNum,
        const FOnlineAccountCredential &AccountCredentials);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineIdentitySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLoginComplete;
    void HandleCallback_OnLoginComplete(
        int32 LocalUserNumCb,
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &Error);

    UPROPERTY()
    int32 __Store__LocalUserNum;

    UPROPERTY()
    FOnlineAccountCredential __Store__AccountCredentials;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineIdentitySubsystemLogoutCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemLogout : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemLogoutCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemLogoutCallbackPin OnLogoutComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemLogout *Logout(UOnlineIdentitySubsystem *Subsystem, int32 LocalUserNum);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineIdentitySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLogoutComplete;
    void HandleCallback_OnLogoutComplete(int32 LocalUserNumCb, bool bWasSuccessful);

    UPROPERTY()
    int32 __Store__LocalUserNum;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlineIdentitySubsystemAutoLoginCallbackPin,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemAutoLogin : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemAutoLoginCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemAutoLoginCallbackPin OnLoginComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemAutoLogin *AutoLogin(UOnlineIdentitySubsystem *Subsystem, int32 LocalUserNum);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineIdentitySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLoginComplete;
    void HandleCallback_OnLoginComplete(
        int32 LocalUserNumCb,
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &Error);

    UPROPERTY()
    int32 __Store__LocalUserNum;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineIdentitySubsystemRevokeAuthTokenCallbackPin,
    const FUniqueNetIdRepl &,
    UserId,
    const FOnlineErrorInfo &,
    OnlineError);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemRevokeAuthToken : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemRevokeAuthTokenCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemRevokeAuthTokenCallbackPin OnRevokeAuthTokenComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemRevokeAuthToken *RevokeAuthToken(
        UOnlineIdentitySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineIdentitySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnRevokeAuthTokenComplete(const FUniqueNetId &UserId, const FOnlineError &OnlineError);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlineIdentitySubsystemGetUserPrivilegeCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    EOnlineUserPrivilege,
    Privilege,
    int64,
    PrivilegeResult);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemGetUserPrivilege : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemGetUserPrivilegeCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemGetUserPrivilegeCallbackPin OnGetUserPrivilegeComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemGetUserPrivilege *GetUserPrivilege(
        UOnlineIdentitySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        EOnlineUserPrivilege Privilege,
        EShowPrivilegeResolveUI_ ShowResolveUI);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineIdentitySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnGetUserPrivilegeComplete(
        const FUniqueNetId &LocalUserId,
        EUserPrivileges::Type Privilege,
        uint32 PrivilegeResult);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    EOnlineUserPrivilege __Store__Privilege;

    UPROPERTY()
    EShowPrivilegeResolveUI_ __Store__ShowResolveUI;
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS