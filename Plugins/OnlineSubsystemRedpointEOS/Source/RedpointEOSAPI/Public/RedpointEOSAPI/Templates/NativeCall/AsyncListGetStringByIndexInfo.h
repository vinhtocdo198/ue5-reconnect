// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/SDK.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TAsyncListGetStringByIndexInfo;
template <typename Handle, typename GetOptions>
struct TAsyncListGetStringByIndexInfo<EOS_EResult(Handle, const GetOptions *, char *, int32_t *)>
{
    using GetOptionsType = GetOptions;
};

}

REDPOINT_EOS_CODE_GUARD_END()