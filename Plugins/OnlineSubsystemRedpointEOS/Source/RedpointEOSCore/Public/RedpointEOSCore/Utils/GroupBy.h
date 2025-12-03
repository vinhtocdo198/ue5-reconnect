// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "Templates/Function.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4277959105, Redpoint::EOS::Core::Utils)
{

template <typename TKey, typename TIn>
TMap<TKey, TArray<TIn>> GroupBy(TArray<TIn> Array, TFunction<TKey(const TIn &)> KeyMapper)
{
    TMap<TKey, TArray<TIn>> Result;
    for (const auto &Val : Array)
    {
        auto Key = KeyMapper(Val);
        if (!Result.Contains(Key))
        {
            Result.Add(Key, TArray<TIn>());
        }
        Result[Key].Add(Val);
    }
    return Result;
}

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(4277959105, Redpoint::EOS::Core::Utils, GroupBy)
}

REDPOINT_EOS_CODE_GUARD_END()