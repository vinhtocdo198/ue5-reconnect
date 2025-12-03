// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/Templates/Util/Cdecl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TAsyncWithImmediateErrorInfo;
template <typename Handle, typename Options, typename Result>
struct TAsyncWithImmediateErrorInfo<
    EOS_EResult(Handle, const Options *, void *, void(__REDPOINT_EOSSDK_CDECL_ATTR *)(const Result *))>
{
    using HandleType = Handle;
    using OptionsType = Options;
    using ResultType = Result;
};

}

REDPOINT_EOS_CODE_GUARD_END()