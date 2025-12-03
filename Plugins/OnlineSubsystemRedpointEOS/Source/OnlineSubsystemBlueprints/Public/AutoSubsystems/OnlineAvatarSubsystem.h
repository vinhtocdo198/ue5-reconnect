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
#include "RedpointEOSInterfaces/Interfaces/OnlineAvatarInterface.h"
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

#include "OnlineAvatarSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAvatarSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlineAvatar, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineAvatar, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineAvatar, ESPMode::ThreadSafe> &InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlineAvatarSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Avatar")
    bool IsSubsystemAvailable();

    friend class UOnlineAvatarSubsystemGetAvatar;

    friend class UOnlineAvatarSubsystemGetAvatarUrl;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineAvatarSubsystemGetAvatarCallbackPin,
    bool,
    bWasSuccessful,
    UTexture *,
    ResultTexture);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAvatarSubsystemGetAvatar : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineAvatarSubsystemGetAvatarCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineAvatarSubsystemGetAvatarCallbackPin OnGetAvatarComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Avatar", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineAvatarSubsystemGetAvatar *GetAvatar(
        UOnlineAvatarSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const FUniqueNetIdRepl &TargetUserId,
        UTexture *DefaultTexture);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineAvatarSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnGetAvatarComplete(bool bWasSuccessful, TSoftObjectPtr<UTexture> ResultTexture);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    FUniqueNetIdRepl __Store__TargetUserId;

    UPROPERTY()
    TObjectPtr<UTexture> __Store__DefaultTexture;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineAvatarSubsystemGetAvatarUrlCallbackPin,
    bool,
    bWasSuccessful,
    const FString &,
    ResultAvatarUrl);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAvatarSubsystemGetAvatarUrl : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineAvatarSubsystemGetAvatarUrlCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineAvatarSubsystemGetAvatarUrlCallbackPin OnGetAvatarUrlComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Avatar", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineAvatarSubsystemGetAvatarUrl *GetAvatarUrl(
        UOnlineAvatarSubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const FUniqueNetIdRepl &TargetUserId,
        const FString &DefaultAvatarUrl);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineAvatarSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnGetAvatarUrlComplete(bool bWasSuccessful, FString ResultAvatarUrl);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    FUniqueNetIdRepl __Store__TargetUserId;

    UPROPERTY()
    FString __Store__DefaultAvatarUrl;
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS