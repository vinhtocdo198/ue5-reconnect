// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TSyncInfo;
template <typename Handle, typename Options, typename Result> struct TSyncInfo<Result(Handle, const Options *)>
{
    using HandleType = Handle;
    using OptionsType = Options;
    using ResultType = Result;
};

}

REDPOINT_EOS_CODE_GUARD_END()