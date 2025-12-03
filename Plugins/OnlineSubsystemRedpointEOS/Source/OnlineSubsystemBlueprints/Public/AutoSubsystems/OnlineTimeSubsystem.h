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
#include "Interfaces/OnlineTimeInterface.h"
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

#include "OnlineTimeSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FTime_QueryServerUtcTimeComplete_BP,
    bool,
    Param1,
    const FString &,
    Param2,
    const FString &,
    Param3);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineTimeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext *WorldContext;
    TWeakPtr<class IOnlineTime, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineTime, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineTime, ESPMode::ThreadSafe> &InHandle);
    FDelegateHandle DelegateHandle_OnQueryServerUtcTimeComplete;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    static FName OSSNameToAssignDuringConstruction;
    FName OSSName = NAME_Default;
    UOnlineTimeSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Time")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Time")
    FTime_QueryServerUtcTimeComplete_BP OnQueryServerUtcTimeComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Time")
    bool QueryServerUtcTime();

    UFUNCTION(BlueprintCallable, Category = "Online|Time")
    FString GetLastServerUtcTime();
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS