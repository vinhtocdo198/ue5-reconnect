// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TSyncListGetByIndexInfo;
template <typename Handle, typename GetOptions, typename ResultEntry>
struct TSyncListGetByIndexInfo<ResultEntry(Handle, const GetOptions *)>
{
    using GetOptionsType = GetOptions;
    using ResultEntryType = ResultEntry;
};

}

REDPOINT_EOS_CODE_GUARD_END()