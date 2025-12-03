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
#include "Interfaces/OnlineChatInterface.h"
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

#include "OnlineChatSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FChat_ChatRoomCreated_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    bool,
    Param3,
    const FString &,
    Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FChat_ChatRoomConfigured_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    bool,
    Param3,
    const FString &,
    Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FChat_ChatRoomJoinPublic_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    bool,
    Param3,
    const FString &,
    Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FChat_ChatRoomJoinPrivate_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    bool,
    Param3,
    const FString &,
    Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FChat_ChatRoomExit_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    bool,
    Param3,
    const FString &,
    Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FChat_ChatRoomMemberJoin_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    const FUniqueNetIdRepl &,
    Param3);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FChat_ChatRoomMemberExit_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    const FUniqueNetIdRepl &,
    Param3);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FChat_ChatRoomMemberUpdate_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    const FUniqueNetIdRepl &,
    Param3);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FChat_ChatRoomMessageSent_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    bool,
    Param3,
    const FString &,
    Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FChat_ChatRoomMessageReceived_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FString &,
    Param2,
    const FChatMessageBP &,
    Param3);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FChat_ChatPrivateMessageSent_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FUniqueNetIdRepl &,
    Param2,
    bool,
    Param3,
    const FString &,
    Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FChat_ChatPrivateMessageReceived_BP,
    const FUniqueNetIdRepl &,
    Param1,
    const FChatMessageBP &,
    Param2);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineChatSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlineChat, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> &InHandle);
    FDelegateHandle DelegateHandle_OnChatRoomCreated;
    FDelegateHandle DelegateHandle_OnChatRoomConfigured;
    FDelegateHandle DelegateHandle_OnChatRoomJoinPublic;
    FDelegateHandle DelegateHandle_OnChatRoomJoinPrivate;
    FDelegateHandle DelegateHandle_OnChatRoomExit;
    FDelegateHandle DelegateHandle_OnChatRoomMemberJoin;
    FDelegateHandle DelegateHandle_OnChatRoomMemberExit;
    FDelegateHandle DelegateHandle_OnChatRoomMemberUpdate;
    FDelegateHandle DelegateHandle_OnChatRoomMessageSent;
    FDelegateHandle DelegateHandle_OnChatRoomMessageReceived;
    FDelegateHandle DelegateHandle_OnChatPrivateMessageSent;
    FDelegateHandle DelegateHandle_OnChatPrivateMessageReceived;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlineChatSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Chat")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomCreated_BP OnChatRoomCreated;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomConfigured_BP OnChatRoomConfigured;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomJoinPublic_BP OnChatRoomJoinPublic;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomJoinPrivate_BP OnChatRoomJoinPrivate;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomExit_BP OnChatRoomExit;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomMemberJoin_BP OnChatRoomMemberJoin;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomMemberExit_BP OnChatRoomMemberExit;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomMemberUpdate_BP OnChatRoomMemberUpdate;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomMessageSent_BP OnChatRoomMessageSent;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomMessageReceived_BP OnChatRoomMessageReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatPrivateMessageSent_BP OnChatPrivateMessageSent;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatPrivateMessageReceived_BP OnChatPrivateMessageReceived;

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool CreateRoom(
        const FUniqueNetIdRepl &UserId,
        const FString &RoomId,
        const FString &Nickname,
        const FChatRoomConfigBP &ChatRoomConfig);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool ConfigureRoom(const FUniqueNetIdRepl &UserId, const FString &RoomId, const FChatRoomConfigBP &ChatRoomConfig);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool JoinPublicRoom(
        const FUniqueNetIdRepl &UserId,
        const FString &RoomId,
        const FString &Nickname,
        const FChatRoomConfigBP &ChatRoomConfig);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool JoinPrivateRoom(
        const FUniqueNetIdRepl &UserId,
        const FString &RoomId,
        const FString &Nickname,
        const FChatRoomConfigBP &ChatRoomConfig);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool ExitRoom(const FUniqueNetIdRepl &UserId, const FString &RoomId);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool SendRoomChat(const FUniqueNetIdRepl &UserId, const FString &RoomId, const FString &MsgBody);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool SendPrivateChat(const FUniqueNetIdRepl &UserId, const FUniqueNetIdRepl &RecipientId, const FString &MsgBody);

    UFUNCTION(BlueprintPure, Category = "Online|Chat")
    bool IsChatAllowed(const FUniqueNetIdRepl &UserId, const FUniqueNetIdRepl &RecipientId);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    void GetJoinedRooms(const FUniqueNetIdRepl &UserId, TArray<FString> &OutRooms);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    FChatRoomInfoBP GetRoomInfo(const FUniqueNetIdRepl &UserId, const FString &RoomId);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool GetMembers(const FUniqueNetIdRepl &UserId, const FString &RoomId, TArray<FChatRoomMemberBP> &OutMembers);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    FChatRoomMemberBP GetMember(
        const FUniqueNetIdRepl &UserId,
        const FString &RoomId,
        const FUniqueNetIdRepl &MemberId);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool GetLastMessages(
        const FUniqueNetIdRepl &UserId,
        const FString &RoomId,
        int32 NumMessages,
        TArray<FChatMessageBP> &OutMessages);

    // @generator-incompatible
    // bool IsMessageFromLocalUser(const FUniqueNetId& UserId, const FChatMessage& Message, const bool
    // bIncludeExternalInstances);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    void DumpChatState();
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS