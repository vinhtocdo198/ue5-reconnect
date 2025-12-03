// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTCAudio/SetInputDeviceSettings.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(SetInputDeviceSettings);

void FSetInputDeviceSettings::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.bPlatformAEC = *Options.bEnablePlatformAEC ? EOS_TRUE : EOS_FALSE;
    NativeOptions.RealDeviceId = Allocator.AsAnsi(Options.DeviceId);
}

void FSetInputDeviceSettings::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

}

REDPOINT_EOS_CODE_GUARD_END()