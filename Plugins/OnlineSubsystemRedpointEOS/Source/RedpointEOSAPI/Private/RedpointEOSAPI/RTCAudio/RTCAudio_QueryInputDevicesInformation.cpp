// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTCAudio/QueryInputDevicesInformation.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_IMPL(QueryInputDevicesInformation);

void FQueryInputDevicesInformation::MapQueryOptions(
    NativeQueryOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FQueryInputDevicesInformation::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FQueryInputDevicesInformation::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.DeviceIndex = Index;
}

void FQueryInputDevicesInformation::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.Id = Converter.FromAnsi(NativeResult.DeviceId);
    Result.DisplayName = Converter.FromUtf8(NativeResult.DeviceName);
    Result.bIsDefaultDevice = NativeResult.bDefaultDevice == EOS_TRUE;
    Result.bIsInputDevice = true;
}

}

REDPOINT_EOS_CODE_GUARD_END()