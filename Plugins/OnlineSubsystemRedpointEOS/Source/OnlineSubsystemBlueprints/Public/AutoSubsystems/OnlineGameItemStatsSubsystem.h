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
#include "Interfaces/OnlineGameItemStatsInterface.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OnlineError.h"
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

#include "OnlineGameItemStatsSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameItemStatsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlineGameItemStats, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineGameItemStats, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineGameItemStats, ESPMode::ThreadSafe> &InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlineGameItemStatsSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|GameItemStats")
    bool IsSubsystemAvailable();

    friend class UOnlineGameItemStatsSubsystemItemUsage;

    friend class UOnlineGameItemStatsSubsystemItemImpact;

    friend class UOnlineGameItemStatsSubsystemItemMitigation;

    friend class UOnlineGameItemStatsSubsystemItemAvailabilityChange;

    friend class UOnlineGameItemStatsSubsystemItemInventoryChange;

    friend class UOnlineGameItemStatsSubsystemItemLoadoutChange;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineGameItemStatsSubsystemItemUsageCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Status);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameItemStatsSubsystemItemUsage : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemUsageCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemUsageCallbackPin OnItemUsageComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|GameItemStats", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineGameItemStatsSubsystemItemUsage *ItemUsage(
        UOnlineGameItemStatsSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const FString &ItemUsedBy,
        const TArray<FString> &ItemsUsed);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineGameItemStatsSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnItemUsageComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Status);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    FString __Store__ItemUsedBy;

    UPROPERTY()
    TArray<FString> __Store__ItemsUsed;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineGameItemStatsSubsystemItemImpactCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Status);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameItemStatsSubsystemItemImpact : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemImpactCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemImpactCallbackPin OnItemImpactComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|GameItemStats", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineGameItemStatsSubsystemItemImpact *ItemImpact(
        UOnlineGameItemStatsSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const TArray<FString> &TargetActors,
        const FString &ImpactInitiatedBy,
        const TArray<FString> &ItemsUsed);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineGameItemStatsSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnItemImpactComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Status);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TArray<FString> __Store__TargetActors;

    UPROPERTY()
    FString __Store__ImpactInitiatedBy;

    UPROPERTY()
    TArray<FString> __Store__ItemsUsed;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineGameItemStatsSubsystemItemMitigationCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Status);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameItemStatsSubsystemItemMitigation : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemMitigationCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemMitigationCallbackPin OnItemMitigationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|GameItemStats", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineGameItemStatsSubsystemItemMitigation *ItemMitigation(
        UOnlineGameItemStatsSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const TArray<FString> &ItemsUsed,
        const TArray<FString> &ImpactItemsMitigated,
        const FString &ItemUsedBy);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineGameItemStatsSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnItemMitigationComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Status);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TArray<FString> __Store__ItemsUsed;

    UPROPERTY()
    TArray<FString> __Store__ImpactItemsMitigated;

    UPROPERTY()
    FString __Store__ItemUsedBy;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineGameItemStatsSubsystemItemAvailabilityChangeCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Status);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameItemStatsSubsystemItemAvailabilityChange
    : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemAvailabilityChangeCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemAvailabilityChangeCallbackPin OnItemAvailabilityChangeComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|GameItemStats", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineGameItemStatsSubsystemItemAvailabilityChange *ItemAvailabilityChange(
        UOnlineGameItemStatsSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const TArray<FString> &AvailableItems,
        const TArray<FString> &UnavailableItems);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineGameItemStatsSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnItemAvailabilityChangeComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Status);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TArray<FString> __Store__AvailableItems;

    UPROPERTY()
    TArray<FString> __Store__UnavailableItems;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineGameItemStatsSubsystemItemInventoryChangeCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Status);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameItemStatsSubsystemItemInventoryChange : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemInventoryChangeCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemInventoryChangeCallbackPin OnItemInventoryChangeComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|GameItemStats", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineGameItemStatsSubsystemItemInventoryChange *ItemInventoryChange(
        UOnlineGameItemStatsSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const TArray<FString> &ItemsToAdd,
        const TArray<FString> &ItemsToRemove);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineGameItemStatsSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnItemInventoryChangeComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Status);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TArray<FString> __Store__ItemsToAdd;

    UPROPERTY()
    TArray<FString> __Store__ItemsToRemove;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineGameItemStatsSubsystemItemLoadoutChangeCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Status);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameItemStatsSubsystemItemLoadoutChange : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemLoadoutChangeCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineGameItemStatsSubsystemItemLoadoutChangeCallbackPin OnItemLoadoutChangeComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|GameItemStats", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineGameItemStatsSubsystemItemLoadoutChange *ItemLoadoutChange(
        UOnlineGameItemStatsSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const TArray<FString> &EquippedItems,
        const TArray<FString> &UnequippedItems);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineGameItemStatsSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnItemLoadoutChangeComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Status);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TArray<FString> __Store__EquippedItems;

    UPROPERTY()
    TArray<FString> __Store__UnequippedItems;
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS