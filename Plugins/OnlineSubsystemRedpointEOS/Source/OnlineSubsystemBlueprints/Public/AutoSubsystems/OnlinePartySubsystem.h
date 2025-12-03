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
#include "Interfaces/OnlinePartyInterface.h"
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

#include "RedpointOSSv1OnlinePartyRequestToJoinInfo.h"
#include "RedpointOSSv1PartyRequestToJoinRemovedReason.h"

#include "OnlinePartySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FParty_PartyJoined_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FParty_PartyExited_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FParty_PartyStateChanged_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    EPartyState_,
    State,
    EPartyState_,
    PreviousState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FParty_PartyPromotionLockoutChanged_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    bool,
    bLockoutState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FParty_PartyConfigChanged_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FOnlinePartyConfiguration &,
    PartyConfig);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FParty_PartyDataReceived_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    FName,
    Namespace,
    UReadablePartyData *,
    PartyData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FParty_PartyMemberPromoted_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    NewLeaderId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FParty_PartyMemberExited_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    MemberId,
    EMemberExitedReason_,
    Reason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FParty_PartyMemberJoined_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    MemberId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
    FParty_PartyMemberDataReceived_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    MemberId,
    FName,
    Namespace,
    UReadablePartyData *,
    PartyData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FParty_PartyInvitesChanged_BP, const FUniqueNetIdRepl &, LocalUserId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FParty_PartyInviteRequestReceived_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    SenderId,
    const FUniqueNetIdRepl &,
    RequestForId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FParty_PartyInviteReceivedEx_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UOnlinePartyJoinInfo *,
    Invitation);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyInviteRemovedEx_BP, const FUniqueNetId&, LocalUserId,
// const IOnlinePartyJoinInfo&, Invitation, EPartyInvitationRemovedReason, Reason);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FParty_PartyInviteResponseReceived_BP, const FUniqueNetId&,
// LocalUserId, const FOnlinePartyId&, PartyId, const FUniqueNetId&, SenderId, const EInvitationResponse, Response);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyJoinRequestReceived_BP, const FUniqueNetId&, LocalUserId,
// const FOnlinePartyId&, PartyId, const IOnlinePartyPendingJoinRequestInfo&, JoinRequestInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FParty_FillPartyJoinRequestData_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    UMutablePartyData *,
    PartyData);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyAnalyticsEvent_BP, const FUniqueNetId&, LocalUserId, const
// FString&, EventName, const TArray<FAnalyticsEventAttribute>&, Attributes);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FParty_PartySystemStateChange_BP, EPartySystemState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FParty_PartyRequestToJoinReceived_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    RequesterId,
    URedpointOSSv1OnlinePartyRequestToJoinInfo *,
    Request);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
    FParty_PartyRequestToJoinRemoved_BP,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    RequesterId,
    URedpointOSSv1OnlinePartyRequestToJoinInfo *,
    Request,
    ERedpointOSSv1PartyRequestToJoinRemovedReason,
    Reason);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> &InHandle);
    FDelegateHandle DelegateHandle_OnPartyJoined;
    FDelegateHandle DelegateHandle_OnPartyExited;
    FDelegateHandle DelegateHandle_OnPartyStateChanged;
    FDelegateHandle DelegateHandle_OnPartyPromotionLockoutChanged;
    FDelegateHandle DelegateHandle_OnPartyConfigChanged;
    FDelegateHandle DelegateHandle_OnPartyDataReceived;
    FDelegateHandle DelegateHandle_OnPartyMemberPromoted;
    FDelegateHandle DelegateHandle_OnPartyMemberExited;
    FDelegateHandle DelegateHandle_OnPartyMemberJoined;
    FDelegateHandle DelegateHandle_OnPartyMemberDataReceived;
    FDelegateHandle DelegateHandle_OnPartyInvitesChanged;
    FDelegateHandle DelegateHandle_OnPartyInviteRequestReceived;
    FDelegateHandle DelegateHandle_OnPartyInviteReceivedEx;
    FDelegateHandle DelegateHandle_OnFillPartyJoinRequestData;
    FDelegateHandle DelegateHandle_OnPartyRequestToJoinReceived;
    FDelegateHandle DelegateHandle_OnPartyRequestToJoinRemoved;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlinePartySubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyJoined_BP OnPartyJoined;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyExited_BP OnPartyExited;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyStateChanged_BP OnPartyStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyPromotionLockoutChanged_BP OnPartyPromotionLockoutChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyConfigChanged_BP OnPartyConfigChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyDataReceived_BP OnPartyDataReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyMemberPromoted_BP OnPartyMemberPromoted;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyMemberExited_BP OnPartyMemberExited;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyMemberJoined_BP OnPartyMemberJoined;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyMemberDataReceived_BP OnPartyMemberDataReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyInvitesChanged_BP OnPartyInvitesChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyInviteRequestReceived_BP OnPartyInviteRequestReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyInviteReceivedEx_BP OnPartyInviteReceivedEx;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_FillPartyJoinRequestData_BP OnFillPartyJoinRequestData;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyRequestToJoinReceived_BP OnPartyRequestToJoinReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyRequestToJoinRemoved_BP OnPartyRequestToJoinRemoved;

    friend class UOnlinePartySubsystemRestoreParties;

    friend class UOnlinePartySubsystemRestoreInvites;

    friend class UOnlinePartySubsystemCleanupParties;

    friend class UOnlinePartySubsystemCreateParty;

    friend class UOnlinePartySubsystemUpdateParty;

    friend class UOnlinePartySubsystemJoinParty;

    // @result-delegates-not-compatible: FOnRequestToJoinPartyComplete(const FUniqueNetId& LocalUserId, const
    // FUniqueNetId& PartyLeaderId, const FDateTime& ExpiresAt, const ERequestToJoinPartyCompletionResult Result) void
    // RequestToJoinParty(const FUniqueNetIdRepl& LocalUserId, int64 PartyTypeId, const FUniqueNetIdRepl& Recipient);

    // @generator-incompatible
    // void ClearRequestToJoinParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId&
    // Sender, EPartyRequestToJoinRemovedReason Reason);

    friend class UOnlinePartySubsystemRejoinParty;

    // @generator-excluded-with-pick
    // bool LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId);

    friend class UOnlinePartySubsystemLeaveParty;

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool ApproveJoinRequest(
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        const FUniqueNetIdRepl &RecipientId,
        bool bIsApproved,
        int32 DeniedResultCode);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    void RespondToQueryJoinability(
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        const FUniqueNetIdRepl &RecipientId,
        bool bCanJoin,
        int32 DeniedResultCode,
        UReadablePartyData *PartyData);

    friend class UOnlinePartySubsystemSendInvitation;

    friend class UOnlinePartySubsystemCancelInvitation;

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool RejectInvitation(const FUniqueNetIdRepl &LocalUserId, const FUniqueNetIdRepl &SenderId);

    // @generator-incompatible
    // void ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId*
    // PartyId);

    friend class UOnlinePartySubsystemKickMember;

    friend class UOnlinePartySubsystemPromoteMember;

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool UpdatePartyData(
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        FName Namespace,
        UReadablePartyData *PartyData);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool UpdatePartyMemberData(
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        FName Namespace,
        UReadablePartyData *PartyMemberData);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool IsMemberLeader(const FUniqueNetIdRepl &LocalUserId, UPartyId *PartyId, const FUniqueNetIdRepl &MemberId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    int64 GetPartyMemberCount(const FUniqueNetIdRepl &LocalUserId, UPartyId *PartyId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UParty *GetParty(const FUniqueNetIdRepl &LocalUserId, UPartyId *PartyId);

    // @generator-excluded-with-pick
    // FOnlinePartyConstPtr GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId& PartyTypeId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UBlueprintPartyMember *GetPartyMember(
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        const FUniqueNetIdRepl &MemberId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UReadablePartyData *GetPartyData(const FUniqueNetIdRepl &LocalUserId, UPartyId *PartyId, FName Namespace);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UReadablePartyData *GetPartyMemberData(
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        const FUniqueNetIdRepl &MemberId,
        FName Namespace);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UOnlinePartyJoinInfo *GetAdvertisedParty(
        const FUniqueNetIdRepl &LocalUserId,
        const FUniqueNetIdRepl &UserId,
        int64 PartyTypeId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetJoinedParties(const FUniqueNetIdRepl &LocalUserId, TArray<UPartyId *> &OutPartyIdArray);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetPartyMembers(
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        TArray<UBlueprintPartyMember *> &OutPartyMembersArray);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetPendingInvites(const FUniqueNetIdRepl &LocalUserId, TArray<UOnlinePartyJoinInfo *> &OutPendingInvitesArray);

    // @generator-incompatible
    // bool GetPendingJoinRequests(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId,
    // TArray<IOnlinePartyPendingJoinRequestInfoConstRef>& OutPendingJoinRequestArray);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetPendingInvitedUsers(
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        TArray<FUniqueNetIdRepl> &OutPendingInvitedUserArray);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetPendingRequestsToJoin(
        const FUniqueNetIdRepl &LocalUserId,
        TArray<URedpointOSSv1OnlinePartyRequestToJoinInfo *> &RequestsToJoin);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    FString MakeJoinInfoJson(const FUniqueNetIdRepl &LocalUserId, UPartyId *PartyId);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    UOnlinePartyJoinInfo *MakeJoinInfoFromJson(const FString &JoinInfoJson);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    FString MakeTokenFromJoinInfo(UOnlinePartyJoinInfo *JoinInfo);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    UOnlinePartyJoinInfo *MakeJoinInfo(const FUniqueNetIdRepl &LocalUserId, UPartyId *PartyId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UOnlinePartyJoinInfo *MakeJoinInfoFromToken(const FString &Token);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    UOnlinePartyJoinInfo *ConsumePendingCommandLineInvite();

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    void DumpPartyState();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlinePartySubsystemRestorePartiesCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemRestoreParties : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRestorePartiesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRestorePartiesCallbackPin OnRestorePartiesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemRestoreParties *RestoreParties(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnRestorePartiesComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlinePartySubsystemRestoreInvitesCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemRestoreInvites : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRestoreInvitesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRestoreInvitesCallbackPin OnRestoreInvitesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemRestoreInvites *RestoreInvites(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnRestoreInvitesComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnlinePartySubsystemCleanupPartiesCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    const FOnlineErrorInfo &,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemCleanupParties : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCleanupPartiesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCleanupPartiesCallbackPin OnCleanupPartiesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemCleanupParties *CleanupParties(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnCleanupPartiesComplete(const FUniqueNetId &LocalUserId, const FOnlineError &Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlinePartySubsystemCreatePartyCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    ECreatePartyCompletionResult_,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemCreateParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCreatePartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCreatePartyCallbackPin OnCreatePartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemCreateParty *CreateParty(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        int64 PartyTypeId,
        const FOnlinePartyConfiguration &PartyConfig);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnCreatePartyComplete(
        const FUniqueNetId &LocalUserId,
        const TSharedPtr<const FOnlinePartyId> &PartyId,
        const ECreatePartyCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    int64 __Store__PartyTypeId;

    UPROPERTY()
    FOnlinePartyConfiguration __Store__PartyConfig;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlinePartySubsystemUpdatePartyCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    EUpdateConfigCompletionResult_,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemUpdateParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemUpdatePartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemUpdatePartyCallbackPin OnUpdatePartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemUpdateParty *UpdateParty(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        const FOnlinePartyConfiguration &PartyConfig,
        bool bShouldRegenerateReservationKey);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnUpdatePartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EUpdateConfigCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TObjectPtr<UPartyId> __Store__PartyId;

    UPROPERTY()
    FOnlinePartyConfiguration __Store__PartyConfig;

    UPROPERTY()
    bool __Store__bShouldRegenerateReservationKey;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FOnlinePartySubsystemJoinPartyCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    EJoinPartyCompletionResult_,
    Result,
    int32,
    NotApprovedReason);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemJoinParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemJoinPartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemJoinPartyCallbackPin OnJoinPartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemJoinParty *JoinParty(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        UOnlinePartyJoinInfo *OnlinePartyJoinInfo);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnJoinPartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EJoinPartyCompletionResult Result,
        const int32 NotApprovedReason);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TObjectPtr<UOnlinePartyJoinInfo> __Store__OnlinePartyJoinInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FOnlinePartySubsystemRejoinPartyCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    EJoinPartyCompletionResult_,
    Result,
    int32,
    NotApprovedReason);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemRejoinParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRejoinPartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRejoinPartyCallbackPin OnJoinPartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemRejoinParty *RejoinParty(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        int64 PartyTypeId,
        const TArray<FUniqueNetIdRepl> &FormerMembers);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnJoinPartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EJoinPartyCompletionResult Result,
        const int32 NotApprovedReason);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TObjectPtr<UPartyId> __Store__PartyId;

    UPROPERTY()
    int64 __Store__PartyTypeId;

    UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__FormerMembers;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnlinePartySubsystemLeavePartyCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    ELeavePartyCompletionResult_,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemLeaveParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemLeavePartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemLeavePartyCallbackPin OnLeavePartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemLeaveParty *LeaveParty(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        bool bSynchronizeLeave);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLeavePartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const ELeavePartyCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TObjectPtr<UPartyId> __Store__PartyId;

    UPROPERTY()
    bool __Store__bSynchronizeLeave;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FOnlinePartySubsystemSendInvitationCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    RecipientId,
    ESendPartyInvitationCompletionResult_,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemSendInvitation : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemSendInvitationCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemSendInvitationCallbackPin OnSendPartyInvitationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemSendInvitation *SendInvitation(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        const FUniqueNetIdRepl &Recipient);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSendPartyInvitationComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &RecipientId,
        const ESendPartyInvitationCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TObjectPtr<UPartyId> __Store__PartyId;

    UPROPERTY()
    FUniqueNetIdRepl __Store__Recipient;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FOnlinePartySubsystemCancelInvitationCallbackPin,
    const FUniqueNetIdRepl &,
    SenderUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    TargetUserId,
    const FOnlineErrorInfo &,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemCancelInvitation : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCancelInvitationCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCancelInvitationCallbackPin OnCancelPartyInvitationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemCancelInvitation *CancelInvitation(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        const FUniqueNetIdRepl &TargetUserId,
        UPartyId *PartyId);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnCancelPartyInvitationComplete(
        const FUniqueNetId &SenderUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &TargetUserId,
        const FOnlineError &Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    FUniqueNetIdRepl __Store__TargetUserId;

    UPROPERTY()
    TObjectPtr<UPartyId> __Store__PartyId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FOnlinePartySubsystemKickMemberCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    MemberId,
    EKickMemberCompletionResult_,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemKickMember : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemKickMemberCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemKickMemberCallbackPin OnKickPartyMemberComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemKickMember *KickMember(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        const FUniqueNetIdRepl &TargetMemberId);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnKickPartyMemberComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const EKickMemberCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TObjectPtr<UPartyId> __Store__PartyId;

    UPROPERTY()
    FUniqueNetIdRepl __Store__TargetMemberId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FOnlinePartySubsystemPromoteMemberCallbackPin,
    const FUniqueNetIdRepl &,
    LocalUserId,
    UPartyId *,
    PartyId,
    const FUniqueNetIdRepl &,
    MemberId,
    EPromoteMemberCompletionResult_,
    Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemPromoteMember : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemPromoteMemberCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemPromoteMemberCallbackPin OnPromotePartyMemberComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemPromoteMember *PromoteMember(
        UOnlinePartySubsystem *Subsystem,
        const FUniqueNetIdRepl &LocalUserId,
        UPartyId *PartyId,
        const FUniqueNetIdRepl &TargetMemberId);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<UOnlinePartySubsystem> __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnPromotePartyMemberComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const EPromoteMemberCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

    UPROPERTY()
    TObjectPtr<UPartyId> __Store__PartyId;

    UPROPERTY()
    FUniqueNetIdRepl __Store__TargetMemberId;
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS