// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Utils/JsonGetOrAdd.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2673256197, Redpoint::EOS::Rooms::Utils)
{
using namespace ::Redpoint::EOS::Rooms::Utils;

TSharedRef<FJsonObject> FJsonGetOrAdd::GetOrAddObjectField(
    const TSharedRef<FJsonObject> &InObject,
    const FString &InKey)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Utils);

    return GetOrAddObjectField(*InObject, InKey);
}

TSharedRef<FJsonObject> FJsonGetOrAdd::GetOrAddObjectField(FJsonObject &InObject, const FString &InKey)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Utils);

    TSharedPtr<FJsonObject> Subobject;
    {
        const TSharedPtr<FJsonObject> *SubobjectPtr = nullptr;
        if (InObject.TryGetObjectField(InKey, SubobjectPtr) && SubobjectPtr->IsValid())
        {
            Subobject = *SubobjectPtr;
        }
        else
        {
            Subobject = MakeShared<FJsonObject>();
            InObject.SetObjectField(InKey, Subobject);
        }
    }
    return Subobject.ToSharedRef();
}

void FJsonGetOrAdd::AddToArrayField(
    const TSharedRef<FJsonObject> &InObject,
    const FString &InKey,
    const TSharedRef<FJsonValue> &InValue)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Utils);

    return AddToArrayField(*InObject, InKey, InValue);
}

void FJsonGetOrAdd::AddToArrayField(
    const TSharedRef<FJsonObject> &InObject,
    const FString &InKey,
    const TArray<TSharedRef<FJsonValue>> &InValues)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Utils);

    return AddToArrayField(*InObject, InKey, InValues);
}

void FJsonGetOrAdd::AddToArrayField(FJsonObject &InObject, const FString &InKey, const TSharedRef<FJsonValue> &InValue)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Utils);

    return AddToArrayField(InObject, InKey, TArray<TSharedRef<FJsonValue>>{InValue});
}

void FJsonGetOrAdd::AddToArrayField(
    FJsonObject &InObject,
    const FString &InKey,
    const TArray<TSharedRef<FJsonValue>> &InValues)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Utils);

    if (!InObject.HasTypedField<EJson::Array>(InKey))
    {
        TArray<TSharedPtr<FJsonValue>> NewArray;
        for (const auto &Value : InValues)
        {
            NewArray.Add(Value);
        }
        InObject.SetArrayField(InKey, NewArray);
    }
    else
    {
        TArray<TSharedPtr<FJsonValue>> MutableCopyOfExistingArray = InObject.GetArrayField(InKey);
        for (const auto &Value : InValues)
        {
            MutableCopyOfExistingArray.Add(Value);
        }
        InObject.SetArrayField(InKey, MutableCopyOfExistingArray);
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()