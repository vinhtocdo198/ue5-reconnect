// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineKeyValuePairDataType.h"
#include "OnlineKeyValuePair.h"

#include "VariantDataBP.generated.h"

/**
 * This type can store any other basic data type, including numbers, strings and boolean values.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Variant Data")) 
struct ONLINESUBSYSTEMBLUEPRINTS_API FVariantDataBP
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    EOnlineKeyValuePairDataType_ Type = EOnlineKeyValuePairDataType_::Empty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool AsBool = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    int32 AsInt = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    float AsFloat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    double AsDouble = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    int64 AsInt64 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString AsString;

    static FVariantDataBP FromNative(const FVariantData &InObj);
    FVariantData ToNative() const;
};