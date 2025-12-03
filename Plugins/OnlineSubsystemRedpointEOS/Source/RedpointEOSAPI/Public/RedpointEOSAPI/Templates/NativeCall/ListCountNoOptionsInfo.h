// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TListCountNoOptionsInfo;
template <typename Handle, typename Count> struct TListCountNoOptionsInfo<Count(Handle)>
{
    using CountType = Count;
};

}

REDPOINT_EOS_CODE_GUARD_END()