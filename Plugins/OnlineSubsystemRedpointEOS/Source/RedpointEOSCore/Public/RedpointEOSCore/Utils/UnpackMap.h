// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4015341085, Redpoint::EOS::Core::Utils)
{

template <typename TKey, typename TValue> TArray<TTuple<TKey, TValue>> UnpackMap(TMap<TKey, TValue> &Map)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    TArray<TTuple<TKey, TValue>> Arr;
    for (const auto &KV : Map)
    {
        Arr.Add(KV);
    }
    return Arr;
}

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(4015341085, Redpoint::EOS::Core::Utils, UnpackMap)
}

REDPOINT_EOS_CODE_GUARD_END()