// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IMessageSanitizerInterface.h"
#include "Online/CoreOnline.h"

#include "OSBBlockedQueryResultInfo.generated.h"

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FBlockedQueryResultInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool Blocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool BlockedNonFriends = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString UserId;

    static FBlockedQueryResultInfo FromNative(const FBlockedQueryResult &InObj);
    FBlockedQueryResult ToNative();
};