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
#include "Interfaces/OnlineUserInterface.h"
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

#include "OnlineUserSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FUser_QueryUserInfoComplete_BP,
    int32,
    LocalUserNum,
    bool,
    bWasSuccessful,
    const TArray<FUniqueNetIdRepl> &,
    UserIds,
    const FString &,
    ErrorStr);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlineUser, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe> &InHandle);
    TArray<FDelegateHandle> DelegateHandle_OnQueryUserInfoComplete;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlineUserSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|User")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|User")
    FUser_QueryUserInfoComplete_BP OnQueryUserInfoComplete;

    friend class UOnlineUserSubsystemQueryUserInfo;

    UFUNCTION(BlueprintCallable, Category = "Online|User")
    bool GetAllUserInfo(int32 LocalUserNum, TArray<UOnlineUserRef *> &OutUsers);

    UFUNCTION(BlueprintCallable, Category = "Online|User")
    UOnlineUserRef *GetUserInfo(int32 LocalUserNum, const FUniqueNetIdRepl &UserId);

    friend class UOnlineUserSubsystemQueryUserIdMapping;

    friend class UOnlineUserSubsystemQueryExternalIdMappings;

    UFUNCTION(BlueprintCallable, Category = "Online|User")
    void GetExternalIdMappings(
        const FExternalIdQueryOptionsBP &QueryOptions,
        const TArray<FString> &ExternalIds,
        TArray<FUniqueNetIdRepl> &OutIds);

    UFUNCTION(BlueprintCallable, Category = "Online|User")
    FUniqueNetIdRepl GetExternalIdMapping(const FExternalIdQueryOptionsBP &QueryOptions, const FString &ExternalId);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlineUserSubsystemQueryUserInfoCallbackPin,
    bool,
    bWasSuccessful,
    const TArray<FUniqueNetIdRepl> &,
    UserIds,
    const FString &,
    ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserSubsystemQueryUserInfo : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryUserInfoCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryUserInfoCallbackPin OnQueryUserInfoComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|User", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserSubsystemQueryUserInfo *QueryUserInfo(
        UOnlineUserSubsystem *Subsystem,
        int32 LocalUserNum,
        const TArray<FUniqueNetIdRepl> &UserIds);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineUserSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnQueryUserInfoComplete;
    void HandleCallback_OnQueryUserInfoComplete(
        int32 LocalUserNumCb,
        bool bWasSuccessful,
        const TArray<FUniqueNetIdRef> &UserIds,
        const FString &ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

    UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__UserIds;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
    FOnlineUserSubsystemQueryUserIdMappingCallbackPin,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    DisplayNameOrEmail,
    const FUniqueNetIdRepl &,
    FoundUserId,
    const FString &,
    Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserSubsystemQueryUserIdMapping : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryUserIdMappingCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryUserIdMappingCallbackPin OnQueryUserMappingComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|User", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserSubsystemQueryUserIdMapping *QueryUserIdMapping(
        UOnlineUserSubsystem *Subsystem,
        const FUniqueNetIdRepl &UserId,
        const FString &DisplayNameOrEmail);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineUserSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnQueryUserMappingComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &DisplayNameOrEmail,
        const FUniqueNetId &FoundUserId,
        const FString &Error);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

    UPROPERTY()
    FString __Store__DisplayNameOrEmail;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
    FOnlineUserSubsystemQueryExternalIdMappingsCallbackPin,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FExternalIdQueryOptionsBP &,
    QueryOptions,
    const TArray<FString> &,
    ExternalIds,
    const FString &,
    Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserSubsystemQueryExternalIdMappings : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryExternalIdMappingsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryExternalIdMappingsCallbackPin OnQueryExternalIdMappingsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|User", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserSubsystemQueryExternalIdMappings *QueryExternalIdMappings(
        UOnlineUserSubsystem *Subsystem,
        const FUniqueNetIdRepl &UserId,
        const FExternalIdQueryOptionsBP &QueryOptions,
        const TArray<FString> &ExternalIds);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineUserSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnQueryExternalIdMappingsComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FExternalIdQueryOptions &QueryOptions,
        const TArray<FString> &ExternalIds,
        const FString &Error);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

    UPROPERTY()
    FExternalIdQueryOptionsBP __Store__QueryOptions;

    UPROPERTY()
    TArray<FString> __Store__ExternalIds;
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS