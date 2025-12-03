// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TListCountInfo;
template <typename Handle, typename Count, typename CountOptions>
struct TListCountInfo<Count(Handle, const CountOptions *)>
{
    using CountType = Count;
    using CountOptionsType = CountOptions;
};

}

REDPOINT_EOS_CODE_GUARD_END()