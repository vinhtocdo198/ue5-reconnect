// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/CreateDeviceId.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(CreateDeviceId);

void FCreateDeviceId::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.DeviceModel = Allocator.AsUtf8(*Options.DeviceModel);
}

void FCreateDeviceId::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

}

REDPOINT_EOS_CODE_GUARD_END()