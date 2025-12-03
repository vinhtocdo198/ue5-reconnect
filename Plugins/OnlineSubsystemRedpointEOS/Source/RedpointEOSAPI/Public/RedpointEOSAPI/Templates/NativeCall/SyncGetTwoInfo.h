// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/SDK.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TSyncGetTwoInfo;
template <typename Handle, typename GetOptions, typename GetResult1, typename GetResult2>
struct TSyncGetTwoInfo<EOS_EResult(Handle, const GetOptions *, GetResult1 *, GetResult2 *)>
{
    using OptionsType = GetOptions;
    using Result1Type = GetResult1;
    using Result2Type = GetResult2;
};

}

REDPOINT_EOS_CODE_GUARD_END()