// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/SDK.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TEventUnregisterInfo;
template <typename Handle> struct TEventUnregisterInfo<void(Handle, EOS_NotificationId)>
{
    using HandleType = Handle;
};

}

REDPOINT_EOS_CODE_GUARD_END()