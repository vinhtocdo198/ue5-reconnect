// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TAsyncListCountInfo;
template <typename Handle, typename CountOptions> struct TAsyncListCountInfo<uint32_t(Handle, const CountOptions *)>
{
    using CountOptionsType = CountOptions;
};

}

REDPOINT_EOS_CODE_GUARD_END()