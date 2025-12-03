// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/SDK.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TAsyncListCopyNoAllocInfo;
template <typename Handle, typename CopyOptions, typename ResultEntry>
struct TAsyncListCopyNoAllocInfo<EOS_EResult(Handle, const CopyOptions *, ResultEntry *)>
{
    using CopyOptionsType = CopyOptions;
    using ResultEntryType = ResultEntry;
};

}

REDPOINT_EOS_CODE_GUARD_END()