// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Containers/Set.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2101967624, Redpoint::EOS::Core::Utils)
{

template <typename TValue> TArray<TValue> SetToArray(const TSet<TValue> &Set)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    TArray<TValue> Result;
    for (const auto &Value : Set)
    {
        Result.Add(Value);
    }
    return Result;
}

template <typename TValue> TSet<TValue> ArrayToSet(const TArray<TValue> &Array)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    TSet<TValue> Result;
    for (const auto &Value : Array)
    {
        Result.Add(Value);
    }
    return Result;
}

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(2101967624, Redpoint::EOS::Core::Utils, SetToArray)
REDPOINT_EOS_FILE_NS_EXPORT(2101967624, Redpoint::EOS::Core::Utils, ArrayToSet)
}

REDPOINT_EOS_CODE_GUARD_END()