// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TSyncNoOptionsInfo;
template <typename Handle, typename Result> struct TSyncNoOptionsInfo<Result(Handle)>
{
    using HandleType = Handle;
    using ResultType = Result;
};

}

REDPOINT_EOS_CODE_GUARD_END()