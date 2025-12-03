// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TAsyncListCountErrorableInfo;
template <typename Handle, typename CountOptions>
struct TAsyncListCountErrorableInfo<EOS_EResult(Handle, const CountOptions *, int32_t *OutCount)>
{
    using CountOptionsType = CountOptions;
};

}

REDPOINT_EOS_CODE_GUARD_END()