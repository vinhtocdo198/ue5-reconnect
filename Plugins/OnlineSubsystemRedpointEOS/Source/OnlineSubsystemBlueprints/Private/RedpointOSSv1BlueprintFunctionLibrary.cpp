// Copyright June Rhodes. All Rights Reserved.

#include "RedpointOSSv1BlueprintFunctionLibrary.h"

#include "OnlineSubsystemBlueprintsModule.h"

int64 URedpointOSSv1BlueprintFunctionLibrary::Conv_FVariantDataBPToInt64(const FVariantDataBP &InData)
{
    if (InData.Type != EOnlineKeyValuePairDataType_::Int64)
    {
        UE_LOG(
            LogOnlineBlueprint,
            Error,
            TEXT("Provided variant data is not of 'Int64' type, so a default value will be returned."));
        return 0;
    }

    return InData.AsInt64;
}

FVariantDataBP URedpointOSSv1BlueprintFunctionLibrary::Conv_Int64ToFVariantDataBP(int64 InData)
{
    FVariantDataBP Value;
    Value.Type = EOnlineKeyValuePairDataType_::Int64;
    Value.AsInt64 = InData;
    return Value;
}

double URedpointOSSv1BlueprintFunctionLibrary::Conv_FVariantDataBPToDouble(const FVariantDataBP &InData)
{
    if (InData.Type != EOnlineKeyValuePairDataType_::Double)
    {
        UE_LOG(
            LogOnlineBlueprint,
            Error,
            TEXT("Provided variant data is not of 'Double' type, so a default value will be returned."));
        return 0.0f;
    }

    return InData.AsDouble;
}

FVariantDataBP URedpointOSSv1BlueprintFunctionLibrary::Conv_DoubleToFVariantDataBP(double InData)
{
    FVariantDataBP Value;
    Value.Type = EOnlineKeyValuePairDataType_::Double;
    Value.AsDouble = InData;
    return Value;
}

FString URedpointOSSv1BlueprintFunctionLibrary::Conv_FVariantDataBPToString(const FVariantDataBP &InData)
{
    if (InData.Type != EOnlineKeyValuePairDataType_::String)
    {
        UE_LOG(
            LogOnlineBlueprint,
            Error,
            TEXT("Provided variant data is not of 'String' type, so a default value will be returned."));
        return TEXT("");
    }

    return InData.AsString;
}

FVariantDataBP URedpointOSSv1BlueprintFunctionLibrary::Conv_StringToFVariantDataBP(const FString &InData)
{
    FVariantDataBP Value;
    Value.Type = EOnlineKeyValuePairDataType_::String;
    Value.AsString = InData;
    return Value;
}

bool URedpointOSSv1BlueprintFunctionLibrary::Conv_FVariantDataBPToBool(const FVariantDataBP &InData)
{
    if (InData.Type != EOnlineKeyValuePairDataType_::Bool)
    {
        UE_LOG(
            LogOnlineBlueprint,
            Error,
            TEXT("Provided variant data is not of 'Bool' type, so a default value will be returned."));
        return false;
    }

    return InData.AsBool;
}

FVariantDataBP URedpointOSSv1BlueprintFunctionLibrary::Conv_BoolToFVariantDataBP(bool InData)
{
    FVariantDataBP Value;
    Value.Type = EOnlineKeyValuePairDataType_::Bool;
    Value.AsBool = InData;
    return Value;
}

bool URedpointOSSv1BlueprintFunctionLibrary::EqualEqual_FVariantDataBPFVariantDataBP(
    const FVariantDataBP &InA,
    const FVariantDataBP &InB)
{
    return InA.Type == InB.Type && InA.ToNative() == InB.ToNative();
}