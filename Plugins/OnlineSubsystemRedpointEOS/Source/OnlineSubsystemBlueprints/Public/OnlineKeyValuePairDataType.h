// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "OnlineKeyValuePairDataType.generated.h"

UENUM(BlueprintType, Meta = (ScriptName = "EOnlineKeyValuePairDataType"))
enum class EOnlineKeyValuePairDataType_ : uint8
{
    /** Means the data in the OnlineData value fields should be ignored */
    Empty,

    /** 32 bit integer */
    Int32,

    /** 32 bit unsigned integer */
    UInt32_NotSupported,

    /** 64 bit integer */
    Int64,

    /** 64 bit unsigned integer */
    UInt64_NotSupported,

    /** Double (8 byte) */
    Double,

    /** Unicode string */
    String,

    /** Float (4 byte) */
    Float,

    /** Binary data */
    Blob_NotSupported,

    /** bool data (1 byte) */
    Bool,

    /** Serialized json text */
    Json_NotSupported
};