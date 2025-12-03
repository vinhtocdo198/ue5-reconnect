// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VariantDataBP.h"

#include "RedpointOSSv1BlueprintFunctionLibrary.generated.h"

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API URedpointOSSv1BlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Convert the variant data to a 64-bit integer */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "To Int64 (Variant Data)",
             CompactNodeTitle = "->",
             Keywords = "variant data int64 integer convert",
             BlueprintAutocast,
             IgnoreTypePromotion))
    static int64 Conv_FVariantDataBPToInt64(const FVariantDataBP &InData);

    /** Convert a 64-bit integer to variant data */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "To Variant Data (Int64)",
             CompactNodeTitle = "->",
             Keywords = "variant data int64 integer convert",
             BlueprintAutocast,
             IgnoreTypePromotion))
    static FVariantDataBP Conv_Int64ToFVariantDataBP(int64 InData);

    /** Convert the variant data to a double */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "To Double (Variant Data)",
             CompactNodeTitle = "->",
             Keywords = "variant data double convert",
             BlueprintAutocast,
             IgnoreTypePromotion))
    static double Conv_FVariantDataBPToDouble(const FVariantDataBP &InData);

    /** Convert a double to variant data */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "To Variant Data (Double)",
             CompactNodeTitle = "->",
             Keywords = "variant data double convert",
             BlueprintAutocast,
             IgnoreTypePromotion))
    static FVariantDataBP Conv_DoubleToFVariantDataBP(double InData);

    /** Convert the variant data to a string */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "To String (Variant Data)",
             CompactNodeTitle = "->",
             Keywords = "variant data string text convert",
             BlueprintAutocast,
             IgnoreTypePromotion))
    static FString Conv_FVariantDataBPToString(const FVariantDataBP &InData);

    /** Convert a string to variant data */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "To Variant Data (String)",
             CompactNodeTitle = "->",
             Keywords = "variant data string text convert",
             BlueprintAutocast,
             IgnoreTypePromotion))
    static FVariantDataBP Conv_StringToFVariantDataBP(const FString &InData);

    /** Convert the variant data to a boolean */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "To Boolean (Variant Data)",
             CompactNodeTitle = "->",
             Keywords = "variant data bool boolean convert",
             BlueprintAutocast,
             IgnoreTypePromotion))
    static bool Conv_FVariantDataBPToBool(const FVariantDataBP &InData);

    /** Convert a boolean to variant data */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "To Variant Data (Boolean)",
             CompactNodeTitle = "->",
             Keywords = "variant data bool boolean convert",
             BlueprintAutocast,
             IgnoreTypePromotion))
    static FVariantDataBP Conv_BoolToFVariantDataBP(bool InData);

    /** Returns true if the two variant datas are equal (A == B) */
    UFUNCTION(
        BlueprintPure,
        meta =
            (DisplayName = "Equal Variant Data", CompactNodeTitle = "==", Keywords = "== equal", IgnoreTypePromotion),
        Category = "Online")
    static bool EqualEqual_FVariantDataBPFVariantDataBP(const FVariantDataBP &InA, const FVariantDataBP &InB);
};