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
#include "Interfaces/OnlinePresenceInterface.h"
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

#include "OnlinePresenceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FPresence_PresenceReceived_BP,
    const FUniqueNetIdRepl &,
    UserId,
    const FOnlineUserPresenceData &,
    Presence);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FPresence_PresenceArrayUpdated_BP,
    const FUniqueNetIdRepl &,
    UserId,
    const TArray<FOnlineUserPresenceData> &,
    NewPresenceArray);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePresenceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlinePresence, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe> &InHandle);
    FDelegateHandle DelegateHandle_OnPresenceReceived;
    FDelegateHandle DelegateHandle_OnPresenceArrayUpdated;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlinePresenceSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Presence")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Presence")
    FPresence_PresenceReceived_BP OnPresenceReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Presence")
    FPresence_PresenceArrayUpdated_BP OnPresenceArrayUpdated;

    friend class UOnlinePresenceSubsystemSetPresence;

    // @generator-incompatible
    // void SetPresence(const FUniqueNetId& User, FOnlinePresenceSetPresenceParameters&& Parameters, const
    // FOnPresenceTaskCompleteDelegate& Delegate);

    friend class UOnlinePresenceSubsystemQueryPresence;

    // @generator-duplicate-needs-picking
    // void QueryPresence(const FUniqueNetId& LocalUserId, const TArray<FUniqueNetIdRef>& UserIds);

    UFUNCTION(BlueprintCallable, Category = "Online|Presence")
    EOnlineCachedResult_ GetCachedPresence(const FUniqueNetIdRepl &User, FOnlineUserPresenceData &OutPresence);

    UFUNCTION(BlueprintCallable, Category = "Online|Presence")
    EOnlineCachedResult_ GetCachedPresenceForApp(
        const FUniqueNetIdRepl &LocalUserId,
        const FUniqueNetIdRepl &User,
        const FString &AppId,
        FOnlineUserPresenceData &OutPresence);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlinePresenceSubsystemSetPresenceCallbackPin,
    const FUniqueNetIdRepl &,
    UserId,
    bool,
    bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePresenceSubsystemSetPresence : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePresenceSubsystemSetPresenceCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePresenceSubsystemSetPresenceCallbackPin OnPresenceTaskComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Presence", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePresenceSubsystemSetPresence *SetPresence(
        UOnlinePresenceSubsystem *Subsystem,
        const FUniqueNetIdRepl &User,
        const FOnlineUserPresenceStatusData &Status);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePresenceSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnPresenceTaskComplete(const class FUniqueNetId &UserId, const bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__User;

    UPROPERTY()
    FOnlineUserPresenceStatusData __Store__Status;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlinePresenceSubsystemQueryPresenceCallbackPin,
    const FUniqueNetIdRepl &,
    UserId,
    bool,
    bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePresenceSubsystemQueryPresence : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePresenceSubsystemQueryPresenceCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePresenceSubsystemQueryPresenceCallbackPin OnPresenceTaskComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Presence", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePresenceSubsystemQueryPresence *QueryPresence(
        UOnlinePresenceSubsystem *Subsystem,
        const FUniqueNetIdRepl &User);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePresenceSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnPresenceTaskComplete(const class FUniqueNetId &UserId, const bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__User;
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS