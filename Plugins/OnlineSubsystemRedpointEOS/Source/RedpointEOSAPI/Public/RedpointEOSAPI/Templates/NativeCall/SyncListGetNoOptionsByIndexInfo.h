// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TSyncListGetNoOptionsByIndexInfo;
template <typename Handle, typename Count, typename ResultEntry>
struct TSyncListGetNoOptionsByIndexInfo<ResultEntry(Handle, Count)>
{
    using ResultEntryType = ResultEntry;
};

}

REDPOINT_EOS_CODE_GUARD_END()