// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/SDK.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TSyncGetInfo;
template <typename Handle, typename GetOptions, typename GetResult>
struct TSyncGetInfo<EOS_EResult(Handle, const GetOptions *, GetResult *)>
{
    using OptionsType = GetOptions;
    using ResultType = GetResult;
};

}

REDPOINT_EOS_CODE_GUARD_END()