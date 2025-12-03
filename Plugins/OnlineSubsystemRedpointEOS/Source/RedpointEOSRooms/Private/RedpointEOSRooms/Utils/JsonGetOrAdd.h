// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "Dom/JsonObject.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2673256197, Redpoint::EOS::Rooms::Utils)
{

class REDPOINTEOSROOMS_API FJsonGetOrAdd
{
public:
    static TSharedRef<FJsonObject> GetOrAddObjectField(const TSharedRef<FJsonObject> &InObject, const FString &InKey);
    static TSharedRef<FJsonObject> GetOrAddObjectField(FJsonObject &InObject, const FString &InKey);
    static void AddToArrayField(
        const TSharedRef<FJsonObject> &InObject,
        const FString &InKey,
        const TSharedRef<FJsonValue> &InValue);
    static void AddToArrayField(
        const TSharedRef<FJsonObject> &InObject,
        const FString &InKey,
        const TArray<TSharedRef<FJsonValue>> &InValues);
    static void AddToArrayField(FJsonObject &InObject, const FString &InKey, const TSharedRef<FJsonValue> &InValue);
    static void AddToArrayField(
        FJsonObject &InObject,
        const FString &InKey,
        const TArray<TSharedRef<FJsonValue>> &InValues);
};

}

namespace Redpoint::EOS::Rooms::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(2673256197, Redpoint::EOS::Rooms::Utils, FJsonGetOrAdd)
}

REDPOINT_EOS_CODE_GUARD_END()