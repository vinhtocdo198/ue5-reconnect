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
#include "Interfaces/OnlineUserCloudInterface.h"
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

#include "OnlineUserCloudSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FUserCloud_EnumerateUserFilesComplete_BP,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FUserCloud_ReadUserFileComplete_BP,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FUserCloud_WriteUserFileProgress64_BP,
    int64,
    BytesWritten,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FUserCloud_WriteUserFileComplete_BP,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FUserCloud_WriteUserFileCanceled_BP,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FUserCloud_DeleteUserFileComplete_BP,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    FileName);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FUserCloud_RequestUsageInfoComplete_BP, bool, bWasSuccessful, const
// FUniqueNetId&, UserId, int64, BytesUsed, const TOptional<int64>&, TotalQuota);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUserCloud_UserCloudAnalyticsEvent_BP, const FString&, EventName, const
// TArray<FAnalyticsEventAttribute>&, Attributes);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> &InHandle);
    FDelegateHandle DelegateHandle_OnEnumerateUserFilesComplete;
    FDelegateHandle DelegateHandle_OnReadUserFileComplete;
    FDelegateHandle DelegateHandle_OnWriteUserFileProgress64;
    FDelegateHandle DelegateHandle_OnWriteUserFileComplete;
    FDelegateHandle DelegateHandle_OnWriteUserFileCanceled;
    FDelegateHandle DelegateHandle_OnDeleteUserFileComplete;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlineUserCloudSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|UserCloud")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_EnumerateUserFilesComplete_BP OnEnumerateUserFilesComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_ReadUserFileComplete_BP OnReadUserFileComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_WriteUserFileProgress64_BP OnWriteUserFileProgress64;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_WriteUserFileComplete_BP OnWriteUserFileComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_WriteUserFileCanceled_BP OnWriteUserFileCanceled;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_DeleteUserFileComplete_BP OnDeleteUserFileComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    bool GetFileContents(const FUniqueNetIdRepl &UserId, const FString &FileName, UFileData *&FileContents);

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    bool ClearFiles(const FUniqueNetIdRepl &UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    bool ClearFile(const FUniqueNetIdRepl &UserId, const FString &FileName);

    friend class UOnlineUserCloudSubsystemEnumerateUserFiles;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    void GetUserFileList(const FUniqueNetIdRepl &UserId, TArray<FCloudFileHeaderBP> &UserFiles);

    friend class UOnlineUserCloudSubsystemReadUserFile;

    friend class UOnlineUserCloudSubsystemWriteUserFile;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    void CancelWriteUserFile(const FUniqueNetIdRepl &UserId, const FString &FileName);

    friend class UOnlineUserCloudSubsystemDeleteUserFile;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    bool RequestUsageInfo(const FUniqueNetIdRepl &UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    void DumpCloudState(const FUniqueNetIdRepl &UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    void DumpCloudFileState(const FUniqueNetIdRepl &UserId, const FString &FileName);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlineUserCloudSubsystemEnumerateUserFilesCallbackPin,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystemEnumerateUserFiles : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemEnumerateUserFilesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemEnumerateUserFilesCallbackPin OnEnumerateUserFilesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserCloudSubsystemEnumerateUserFiles *EnumerateUserFiles(
        UOnlineUserCloudSubsystem *Subsystem,
        const FUniqueNetIdRepl &UserId);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineUserCloudSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnEnumerateUserFilesComplete;
    void HandleCallback_OnEnumerateUserFilesComplete(bool bWasSuccessful, const FUniqueNetId &UserId);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlineUserCloudSubsystemReadUserFileCallbackPin,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    FileName);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystemReadUserFile : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemReadUserFileCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemReadUserFileCallbackPin OnReadUserFileComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserCloudSubsystemReadUserFile *ReadUserFile(
        UOnlineUserCloudSubsystem *Subsystem,
        const FUniqueNetIdRepl &UserId,
        const FString &FileName);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineUserCloudSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnReadUserFileComplete;
    void HandleCallback_OnReadUserFileComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &FileName);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

    UPROPERTY()
    FString __Store__FileName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    FileName,
    int64,
    BytesWritten);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystemWriteUserFile : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin OnWriteUserFileComplete;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin OnWriteUserFileProgress64;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin OnWriteUserFileCanceled;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserCloudSubsystemWriteUserFile *WriteUserFile(
        UOnlineUserCloudSubsystem *Subsystem,
        const FUniqueNetIdRepl &UserId,
        const FString &FileName,
        UFileData *FileContents,
        bool bCompressBeforeUpload);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineUserCloudSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnWriteUserFileComplete;
    FDelegateHandle ProgressDelegateHandle_OnWriteUserFileComplete;
    FDelegateHandle CancelDelegateHandle_OnWriteUserFileComplete;
    void HandleCallback_OnWriteUserFileComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &FileName);
#if REDPOINT_EOS_UE_5_4_OR_LATER
    void HandleCallbackProgress_OnWriteUserFileComplete(
        uint64 BytesWritten,
        const FUniqueNetId &UserId,
        const FString &FileName);
#else
    void HandleCallbackProgress_OnWriteUserFileComplete(
        int32 BytesWritten,
        const FUniqueNetId &UserId,
        const FString &FileName);
#endif
    void HandleCallbackCancel_OnWriteUserFileComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &FileName);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

    UPROPERTY()
    FString __Store__FileName;

    UPROPERTY()
    TObjectPtr<UFileData> __Store__FileContents;

    UPROPERTY()
    bool __Store__bCompressBeforeUpload;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlineUserCloudSubsystemDeleteUserFileCallbackPin,
    bool,
    bWasSuccessful,
    const FUniqueNetIdRepl &,
    UserId,
    const FString &,
    FileName);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystemDeleteUserFile : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemDeleteUserFileCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemDeleteUserFileCallbackPin OnDeleteUserFileComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserCloudSubsystemDeleteUserFile *DeleteUserFile(
        UOnlineUserCloudSubsystem *Subsystem,
        const FUniqueNetIdRepl &UserId,
        const FString &FileName,
        bool bShouldCloudDelete,
        bool bShouldLocallyDelete);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlineUserCloudSubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnDeleteUserFileComplete;
    void HandleCallback_OnDeleteUserFileComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &FileName);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

    UPROPERTY()
    FString __Store__FileName;

    UPROPERTY()
    bool __Store__bShouldCloudDelete;

    UPROPERTY()
    bool __Store__bShouldLocallyDelete;
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS