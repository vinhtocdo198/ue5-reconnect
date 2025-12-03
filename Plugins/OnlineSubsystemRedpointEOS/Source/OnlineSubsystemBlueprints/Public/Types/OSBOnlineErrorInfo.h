// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Online/CoreOnline.h"
#include "OnlineError.h"

#include "OSBOnlineErrorInfo.generated.h"

/**
 * Represents the result from an online operation.
 */
USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineErrorInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool Successful = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString ErrorRaw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString ErrorCode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FText ErrorMessage;

    static FOnlineErrorInfo FromNative(const FOnlineError &Obj);
    FOnlineError ToNative();
};