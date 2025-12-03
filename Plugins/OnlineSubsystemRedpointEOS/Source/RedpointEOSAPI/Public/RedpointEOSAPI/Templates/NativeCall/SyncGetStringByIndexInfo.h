// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/SDK.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TSyncGetStringByIndexInfo;
template <typename Handle, typename GetOptions>
struct TSyncGetStringByIndexInfo<EOS_EResult(Handle, const GetOptions *, char *, uint32_t *)>
{
    using GetOptionsType = GetOptions;
};

}

REDPOINT_EOS_CODE_GUARD_END()