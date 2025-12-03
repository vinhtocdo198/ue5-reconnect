// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/Util/Cdecl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TEventRegisterInfo;
template <typename Handle, typename RegisterOptions, typename Result>
struct TEventRegisterInfo<
    EOS_NotificationId(Handle, const RegisterOptions *, void *, void(__REDPOINT_EOSSDK_CDECL_ATTR *)(const Result *))>
{
    using HandleType = Handle;
    using RegisterOptionsType = RegisterOptions;
    using ResultType = Result;
};

}

REDPOINT_EOS_CODE_GUARD_END()